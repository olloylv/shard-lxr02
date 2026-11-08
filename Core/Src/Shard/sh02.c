/*
 * sh02.c - SHARD sound engines in the voice slots of the LXR-02, see sh02.h
 *
 * Copyright (C) 2026 SHARD contributors, based on OpenElixir-02 (C) 2026
 * tunefish.at and the LXR firmware (C) 2012-2026 Erica Synths.
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "Shard/sh02.h"
#include "Soundengine/DSPAudio/VoiceSlot.h"
#include "Soundengine/DSPAudio/ShardVoice.h"
#include "Preset/KitEngines.h"
#include "menu/menu.h"
#include "Parameters.h"
#include <string.h>

_Static_assert(SH02_FIRST_ENGINE + SH02_NUM_ENGINES == NUM_ENGINES, "engine numbers");
_Static_assert(SH02_NUM_PARAMS == 1 + NUM_SLOT_ROLES, "8 engine parameters: waveform + 7 roles");

static const uint8_t sh02_oscWave[NUM_VOICE_SLOTS] =
{
	PAR_OSC_WAVE_DRUM1, PAR_OSC_WAVE_DRUM2, PAR_OSC_WAVE_DRUM3, PAR_OSC_WAVE_SNARE, PAR_WAVE1_CYM, PAR_WAVE1_HH
};

static const char sh02_stockShort[SH02_FIRST_ENGINE][4] = { "drm", "snr", "cym", "hht" };
static const char sh02_stockLong[SH02_FIRST_ENGINE][9] = { "drum", "snare", "cymbal", "hi-hat" };

//---------------------------------------------------
const ShEngineInfo* sh02_info(const uint8_t engine)
{
	return sh_engineInfo((uint8_t)(engine - SH02_FIRST_ENGINE));
}
//---------------------------------------------------
const char* sh02_engineShortName(const uint8_t engine)
{
	if(engine < SH02_FIRST_ENGINE)
		return sh02_stockShort[engine];
	return sh02_info(engine)->shortName;
}
//---------------------------------------------------
const char* sh02_engineLongName(const uint8_t engine)
{
	if(engine < SH02_FIRST_ENGINE)
		return sh02_stockLong[engine];
	return sh02_info(engine)->longName;
}
//---------------------------------------------------
uint16_t sh02_param(const uint8_t slot, const uint8_t k)
{
	if(slot >= NUM_VOICE_SLOTS || k >= SH02_NUM_PARAMS)
		return PAR_NONE;
	return k == 0 ? sh02_oscWave[slot] : slot_rolePar[slot][k-1];
}
//---------------------------------------------------
int8_t sh02_paramIndex(const uint8_t slot, const uint16_t par)
{
	for(uint8_t k=0;k<SH02_NUM_PARAMS;k++)
	{
		if(sh02_param(slot, k) == par)
			return (int8_t)k;
	}
	return -1;
}
//---------------------------------------------------
uint8_t sh02_setParameter(const uint16_t par, const uint8_t itg, const float flt, const uint8_t isFloatModulator)
{
	for(uint8_t slot=0;slot<NUM_VOICE_SLOTS;slot++)
	{
		if(!sh02_isShard(voiceSlots[slot].engine))
			continue;
		const int8_t k = sh02_paramIndex(slot, par);
		if(k < 0)
			continue;
		float v = isFloatModulator ? flt * 127.f : itg;
		if(v < 0.f) v = 0.f;
		if(v > 127.f) v = 127.f;
		Shard_setParam(&voiceSlots[slot].u.shard, (uint8_t)k, (uint8_t)(v + 0.5f));
		return 1;
	}
	return 0;
}
//---------------------------------------------------
static uint8_t sh02_nameSlot = NUM_VOICE_SLOTS;
void sh02_setNameSlot(const uint8_t slot)
{
	sh02_nameSlot = slot;
}
//---------------------------------------------------
uint8_t sh02_modTargetName(const uint16_t par)
{
	const uint8_t slot = kitEngines_slotOfParam(par);
	if(slot >= NUM_VOICE_SLOTS || !sh02_isShard(kitEngines[slot]))
		return 0;
	const int8_t k = sh02_paramIndex(slot, par);
	if(k < 0)
		return 0;
	if(sh02_info(kitEngines[slot])->params[k].type == SH_PT_NONE)
		return 0xFF;
	sh02_nameSlot = slot;
	return (uint8_t)(TEXT_SH_E1 + k);
}
//---------------------------------------------------
uint8_t sh02_nameSlotOrActive(void)
{
	if(sh02_nameSlot < NUM_VOICE_SLOTS)
		return sh02_nameSlot;
	const uint8_t voice = menu_getActiveVoice();
	return voice < NUM_VOICE_SLOTS ? voice : NUM_VOICE_SLOTS-1;
}
//---------------------------------------------------
//the engine parameter description of k for the active voice (UI side)
static const ShParamDesc* sh02_activeDesc(const uint8_t k)
{
	const uint8_t slot = sh02_nameSlotOrActive();
	if(!sh02_isShard(kitEngines[slot]) || k >= SH02_NUM_PARAMS)
		return 0;
	return &sh02_info(kitEngines[slot])->params[k];
}
//---------------------------------------------------
uint16_t sh02_dtype(const uint16_t par)
{
	const uint8_t slot = kitEngines_slotOfParam(par);
	if(slot >= NUM_VOICE_SLOTS || !sh02_isShard(kitEngines[slot]))
		return 0;
	const int8_t k = sh02_paramIndex(slot, par);
	if(k < 0)
		return 0;
	const ShParamDesc* d = &sh02_info(kitEngines[slot])->params[k];
	switch(d->type)
	{
	case SH_PT_LIST:	return (uint16_t)(((MENU_SH_E1 + k) << 8) | DTYPE_MENU);
	case SH_PT_ONOFF:	return DTYPE_ON_OFF;
	case SH_PT_BI:		return DTYPE_PM63;
	default:			return DTYPE_0B127;
	}
}
//---------------------------------------------------
uint8_t sh02_menuCount(const uint8_t k)
{
	const ShParamDesc* d = sh02_activeDesc(k);
	return d && d->type == SH_PT_LIST ? d->count : 1;
}
//---------------------------------------------------
void sh02_menuLabel(const uint8_t k, const uint8_t value, char* buf3)
{
	const ShParamDesc* d = sh02_activeDesc(k);
	if(!d || d->type != SH_PT_LIST || !d->labels || value >= d->count)
	{
		memcpy(buf3, "---", 3);
		return;
	}
	memcpy(buf3, d->labels[value], 3);
}
//---------------------------------------------------
const char* sh02_paramShortName(const uint8_t k)
{
	const ShParamDesc* d = sh02_activeDesc(k);
	return d && d->type != SH_PT_NONE ? d->shortName : "   ";
}
//---------------------------------------------------
const char* sh02_paramLongName(const uint8_t k)
{
	const ShParamDesc* d = sh02_activeDesc(k);
	return d && d->type != SH_PT_NONE ? d->longName : "";
}
//---------------------------------------------------
void sh02_applyDefaults(const uint8_t slot, const uint8_t engine, uint8_t* params)
{
	if(slot >= NUM_VOICE_SLOTS || !sh02_isShard(engine))
		return;
	const ShEngineInfo* info = sh02_info(engine);
	for(uint8_t k=0;k<SH02_NUM_PARAMS;k++)
	{
		const ShParamDesc* d = &info->params[k];
		params[sh02_param(slot, k)] = d->type == SH_PT_NONE ? 0 : d->def;
	}
	const ShEngineDefaults* def = &info->defaults;
	params[PAR_COARSE1 + 2*slot] = def->tune != 0xff ? def->tune : 60;
	params[PAR_FINE1 + 2*slot] = 64;
	//the engines with their own envelope: the amp EG stays open
	const uint8_t own = def->aegMode == SH_AEG_OFF;
	params[PAR_VELOA1 + 2*slot] = own ? 0 : (def->attack != 0xff ? def->attack : 0);
	params[PAR_VELOD1 + 2*slot] = own ? 127 : (def->decay != 0xff ? def->decay : 64);
	if(slot == NUM_VOICE_SLOTS-1)
		params[PAR_VELOD6_OPEN] = params[PAR_VELOD1 + 2*slot];
	//SHARD filter types: off LP HP BP UBP Nch Pk LP2, stock: LP HP BP UBP Nch Pk LP2 off
	if(def->filterType != 0xff)
		params[PAR_FILTER_TYPE_1 + slot] = def->filterType == 0 ? 7 : (uint8_t)(def->filterType - 1);
	if(def->filterFreq != 0xff)
		params[PAR_FILTER_FREQ_1 + slot] = def->filterFreq;
	//no click layer and no pitch modulation by default
	params[PAR_TRANS1_VOL + slot] = 0;
}
