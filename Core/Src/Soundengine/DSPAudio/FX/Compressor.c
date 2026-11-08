/*
 * Compressor.c
 *
 *  Created on: Aug 27, 2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Soundengine/DSPAudio/FX/Compressor.h"
#include "Soundengine/DSPAudio/mixer.h"
#include "math.h"
#include "main.h"


enum
{
	RATIO_1,
	RATIO_2,
	RATIO_3,
	RATIO_4,
	RATIO_6,
	RATIO_8
};

static const double DC_OFFSET = 1.0E-25;

void Compressor_init(Compressor *c)
{
	c->attack = 0.1;
	c->release = 300;

	//c->filter = 0.0;
	c->env = 0.0;
	c->tresh = 3;
	c->markupGain = 1.0f;

}

// linear -> dB conversion
//SHARD: fast approximations for the gain computer (about 0.01 dB), the same
//curve as lin2dB/dB2lin (as in the SHARD Eurorack firmware 0.5)
#define COMP_DB_PER_LOG2 6.0205999f		/* 20 * log10(2) */
static inline float comp_fastLog2(float x)
{
	union { float f; uint32_t i; } v = { x };
	const float e = (float)(int32_t)((v.i >> 23) & 0xFF) - 127.f;
	v.i = (v.i & 0x007FFFFF) | 0x3F800000;	/* mantissa in [1,2) */
	const float m = v.f;
	return e + ((0.15392465f * m - 1.0295584f) * m + 3.010851f) * m - 2.1338866f;
}
static inline float comp_fastExp2(float x)
{
	if(x < -60.f) return 0.f;
	if(x > 60.f) x = 60.f;
	const float fl = floorf(x);
	const float f = x - fl;
	const float p = ((0.07902041f * f + 0.22412837f) * f + 0.69683624f) * f + 0.99981246f;
	union { float f; uint32_t i; } v;
	v.i = (uint32_t)((int32_t)fl + 127) << 23;
	return p * v.f;
}

// dB -> linear conversion

//att/release, time in ms to reach 63% of final value
void Compressor_setAttack(Compressor *c, float a)
{
	c->attack = 0.03f + a *100.f; //[ms]
	c->attack = exp( -1000.0 / ( c->attack * REAL_FS ) );
}
void Compressor_setTreshold(Compressor *c, float t)
{
	c->tresh = -96 + t*96; //[-96:0]dB
}

void Compressor_setGain(Compressor *c, float g)
{
	c->markupGain = 1.f + g*2;
}
void Compressor_setRelease(Compressor *c, float r)
{
	c->release = 25+r*175.f; //[ms]
	c->release = exp( -1000.0 / ( c->release * REAL_FS ) );
}

void Compressor_setRatio(Compressor *c, uint8_t r)
{
	switch(r)
	{
	default:
	case RATIO_1:
		c->ratio = 1.0f;
		break;

	case RATIO_2:
		c->ratio = 0.5f;
		break;
	case RATIO_3:
		c->ratio = 1.f/3.f;
		break;
	case RATIO_4:
		c->ratio = 0.25f;
		break;
	case RATIO_6:
		c->ratio = 1.f/6.f;
		break;
	case RATIO_8:
		c->ratio = 0.125f;
		break;
	}
}

void Compressor_process(Compressor *c, int16_t *fxBuffer,int32_t* outL, int32_t* outR, uint16_t size,  uint8_t channels, float dryWet)
{


	int32_t x;
	int32_t* stereoOut = outL;

	int16_t left[size];
	int16_t right[size];

	if(channels==2)
	{

		for(uint8_t i=0; i<size;i++)
		{
			//--- Compressor ---
			float in = 0.5 * (fxBuffer[i*2]/32768.f) + 0.5 * (fxBuffer[i*2+1]/32768.f);
			double peak = fabs(in);

			//SHARD: float gain computer with fast log2/exp2 (was double logf/expf per sample)
			const float peakdB = COMP_DB_PER_LOG2 * comp_fastLog2((float)peak + 1e-25f);
			float overdB = peakdB - (float)c->tresh;
			if(overdB < 0.f)
				overdB = 0.f;

			// dynamic selection: attack or release?
			const float env = (float)c->env;
			const float coeff = overdB > env ? (float)c->attack : (float)c->release;
			c->env = overdB + coeff * (env - overdB);

			const float gr = c->env > 1e-4 ? comp_fastExp2((float)c->env * ((float)c->ratio - 1.f) * (1.f / COMP_DB_PER_LOG2)) : 1.f;
			//debugWatch = gr;

			left[i]  = __SSAT(fxBuffer[i*2] * gr * c->markupGain, 16);//gain;
			right[i] = __SSAT(fxBuffer[i*2+1] * gr * c->markupGain, 16);//gain;


			//--- Output transfer ---
			//TODO can be optimized quite a bit with SIMD
			const uint8_t idx1 = i*2;
			const uint8_t idx2 = i*2+1;
			//int16_t wet = __SSAT((int32_t)(left[i] * 32768.0f), 16);
			//int16_t wet2 = __SSAT((int32_t)(right[i] * 32768.0f), 16);
			x = __PKHBT(fxBuffer[idx1] + __SSAT((left[i] - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((right[i] - fxBuffer[idx2]) * dryWet,16), 16);
			//int16_t dw = fxBuffer[i] + __SSAT((wet - fxBuffer[i]) * dryWet,16);
			mixer_addStereo(stereoOut, x);
			stereoOut += 2;
		}
	}
	else
	{
		for(uint8_t i=0; i<size;i++)
		{
			const uint8_t idx1 = i*2;
			//--- Compressor ---
			float in = fxBuffer[idx1]/32768.f;
			double peak = fabs(in);

			//SHARD: float gain computer with fast log2/exp2 (was double logf/expf per sample)
			const float peakdB = COMP_DB_PER_LOG2 * comp_fastLog2((float)peak + 1e-25f);
			float overdB = peakdB - (float)c->tresh;
			if(overdB < 0.f)
				overdB = 0.f;

			// dynamic selection: attack or release?
			const float env = (float)c->env;
			const float coeff = overdB > env ? (float)c->attack : (float)c->release;
			c->env = overdB + coeff * (env - overdB);

			const float gr = c->env > 1e-4 ? comp_fastExp2((float)c->env * ((float)c->ratio - 1.f) * (1.f / COMP_DB_PER_LOG2)) : 1.f;
			//debugWatch = gr;

			left[i]  = __SSAT(fxBuffer[idx1] * gr * c->markupGain, 16);//gain; (saturate like the stereo path, it wrapped)


			//--- Output transfer ---
			const int16_t dw = fxBuffer[idx1] + __SSAT((left[i] - fxBuffer[idx1]) * dryWet,16);
			*outL += dw;
			outL += 2;
		}
	}
}

