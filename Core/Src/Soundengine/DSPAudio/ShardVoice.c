/*
 * ShardVoice.c - a voice slot running a SHARD sound engine, see ShardVoice.h
 *
 * Copyright (C) 2026 SHARD contributors, based on OpenElixir-02 (C) 2026
 * tunefish.at and the LXR firmware (C) 2012-2026 Erica Synths.
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "Soundengine/DSPAudio/ShardVoice.h"
#include "Soundengine/DSPAudio/BufferTools.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/MIDI/MidiParser.h"
#include "Soundengine/Sequencer/sequencer.h"
#include "IO/adcPots.h"
#include <math.h>
#include <string.h>

//---------------------------------------------------
void Shard_init(ShardVoice* v, const uint8_t slot, const uint8_t engine)
{
	v->slot = slot;
	initOnePole(&v->FaderVolumeFilter);
	setOnePoleCoef(&v->FaderVolumeFilter, 0.01);
	initOnePole(&v->distortion.DriveParamFilter);
	setOnePoleCoef(&v->distortion.DriveParamFilter, 0.01);
	SnapEg_init(&v->snapEg);
	v->pan = 0;
	v->vol = 0.8f;
	transient_init(&v->transGen);
	SVF_init(&v->filter);
	v->filterType = 0x01;
	lfo_init(&v->lfo);
	v->osc.freq = 440;
	v->osc.modNodeValue = 1;
	v->osc.pitchMod = 1.0f;
	v->osc.fmMod = 0;
	v->osc.midiFreq = 60<<8 | 64;
	v->osc.baseNote = SEQ_DEFAULT_NOTE;
	slopeEg2_init(&v->oscVolEg);
	setDistortionShape(&v->distortion, 0.f);
	v->volumeMod = 1;

	v->engine = engine;
	memset(v->param, 0, sizeof(v->param));
	v->pendingTrigger = 0;
	v->alt = 0;
	v->note = 60.f;
	v->ampFilterInput = 0;
	v->lastGain = 0;
	sh_engineSelect(slot, engine);
}
//---------------------------------------------------
void Shard_setParam(ShardVoice* v, const uint8_t k, const uint8_t value)
{
	if(k < SH02_NUM_PARAMS)
		v->param[k] = value;
}
//---------------------------------------------------
void Shard_trigger(ShardVoice* v, const uint8_t vel, const uint8_t second, const uint8_t note)
{
	lfo_retrigger(v->slot);
	modNode_updateValue(&velocityModulators[v->slot], vel/127.f);
	slopeEg2_trigger(&v->oscVolEg);
	v->velo = vel/127.f;
	v->osc.baseNote = note;
	transient_trigger(&v->transGen);
	SnapEg_trigger(&v->snapEg);
	SVF_reset(&v->filter);
	v->triggeredPanL = sqrtf((127-v->pan)/127.f);
	v->triggeredPanR = sqrtf(v->pan/127.f);
	v->alt = second;
	v->pendingTrigger = 1;
}
//---------------------------------------------------
void Shard_calcAsync(ShardVoice* v)
{
	//amp EG, interpolated over the block like the drum voice
	if((v->oscVolEg.attack == 1) && ((v->oscVolEg.state == EG_A) || (v->oscVolEg.state == EG_REPEAT)))
	{
		v->ampFilterInput = slopeEg2_calc(&v->oscVolEg);
		v->lastGain = v->ampFilterInput;
	}
	else
	{
		v->lastGain = v->ampFilterInput;
		v->ampFilterInput = slopeEg2_calc(&v->oscVolEg);
	}

	//pitch: coarse + fine + the step's note, plus LFO / velocity pitch modulation
	const float fine = midiParser_calcDetune(v->osc.midiFreq & 0xff);
	const float mod = v->osc.pitchMod * v->osc.modNodeValue;
	float note = (float)(v->osc.midiFreq >> 8) + (float)v->osc.baseNote - SEQ_DEFAULT_NOTE;
	note += 12.f * log2f(fine * (mod > 0.0001f ? mod : 0.0001f));
	v->note = note;
}
//---------------------------------------------------
void Shard_calcSyncBlock(ShardVoice* v, int16_t* buf, const uint8_t size)
{
	ShRenderIn in;
	memset(&in, 0, sizeof(in));
	in.note = v->note;
	in.accent = v->velo;
	in.trigger = v->pendingTrigger;
	in.alt = v->alt;
	v->pendingTrigger = 0;

	const ShEngineInfo* info = sh_engineInfo(v->engine);
	for(uint8_t k=0;k<SH02_NUM_PARAMS;k++)
	{
		const uint8_t raw = v->param[k];
		const ShParamDesc* d = &info->params[k];
		switch(d->type)
		{
		case SH_PT_LIST:
			in.idx[k] = raw < d->count ? raw : (uint8_t)(d->count ? d->count - 1 : 0);
			in.p[k] = d->count > 1 ? in.idx[k] / (float)(d->count - 1) : 0.f;
			break;
		case SH_PT_ONOFF:
			in.idx[k] = raw ? 1 : 0;
			in.p[k] = in.idx[k];
			break;
		default:
			in.p[k] = raw * (1.f / 127.f);
			break;
		}
	}

	float out[OUTPUT_DMA_SIZE];
	sh_engineRender(v->slot, &in, out);
	for(uint8_t i=0;i<size;i++)
	{
		float x = out[i] * 32767.f;
		if(x > 32767.f) x = 32767.f;
		if(x < -32768.f) x = -32768.f;
		buf[i] = (int16_t)x;
	}

	//stock chain of the drum voice from here on
	int16_t transBuf[OUTPUT_DMA_SIZE];
	transient_calcBlock(&v->transGen, transBuf, size);
	bufferTool_addBuffersSaturating(buf, transBuf, size);

	SVF_calcBlockZDF(&v->filter, v->filterType, buf, size);

	bufferTool_addGainInterpolated(buf, v->ampFilterInput, v->lastGain, size);

	if(v->volumeMod)
	{
		bufferTool_addGain(buf, v->velo, size);
	}

	calcDistBlockFiltered(&v->distortion, buf, size);

	const float faderVolume = adc_getFaderVolume(v->slot);
	for(uint8_t i=0;i<size;i++)
	{
		v->combinedVolume = v->vol * calcOnePole(&v->FaderVolumeFilter, faderVolume);
		buf[i] *= v->combinedVolume;
	}
}
