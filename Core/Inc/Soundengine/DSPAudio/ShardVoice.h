/*
 * ShardVoice.h - a voice slot running a SHARD sound engine
 *
 * Copyright (C) 2026 SHARD contributors, based on OpenElixir-02 (C) 2026
 * tunefish.at and the LXR firmware (C) 2012-2026 Erica Synths.
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * The SHARD engine replaces the oscillators of the stock voice; everything
 * after them is the stock chain of the drum voice: transient, filter, amp EG,
 * velocity, drive, volume and fader. The engine itself (up to 17 KB of state)
 * lives in the SHARD engine storage of the slot, not in the voice union.
 */
#ifndef SHARDVOICE_H_
#define SHARDVOICE_H_

#include "Soundengine/DSPAudio/VoiceCommon.h"
#include "Shard/sh02.h"

typedef struct
{
	VOICE_COMMON_FIELDS
	// SHARD specific
	uint8_t		engine;					// SHARD engine index (LXR-02 engine - SH02_FIRST_ENGINE)
	uint8_t		param[SH02_NUM_PARAMS];	// engine parameters 0..127
	volatile uint8_t pendingTrigger;		// 1 = trigger at the start of the next block
	uint8_t		alt;					// the trigger came from track 7
	float		note;					// pitch of the next block (MIDI note)
	double		ampFilterInput;
	double		lastGain;
} ShardVoice;

void Shard_init(ShardVoice* v, const uint8_t slot, const uint8_t engine);
void Shard_trigger(ShardVoice* v, const uint8_t vel, const uint8_t second, const uint8_t note);
void Shard_setParam(ShardVoice* v, const uint8_t k, const uint8_t value);
void Shard_calcAsync(ShardVoice* v);
void Shard_calcSyncBlock(ShardVoice* v, int16_t* buf, const uint8_t size);

#endif /* SHARDVOICE_H_ */
