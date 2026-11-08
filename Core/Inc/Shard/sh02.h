/*
 * sh02.h - SHARD sound engines in the voice slots of the LXR-02
 *
 * Copyright (C) 2026 SHARD contributors, based on OpenElixir-02 (C) 2026
 * tunefish.at and the LXR firmware (C) 2012-2026 Erica Synths.
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Engines 0-3 of a voice slot are the stock drum, snare, cymbal and hi-hat.
 * Engine SH02_FIRST_ENGINE + i is SHARD engine i (the same numbers as on the
 * SHARD Eurorack module, which has the 4 LXR classics in front as well).
 *
 * A SHARD voice keeps the stock voice parameters of its slot: pitch (coarse,
 * fine), amp EG, filter, transient, drive, volume, pan, sample rate, LFO,
 * velocity modulation and output work as for the stock engines. Its up to 8
 * engine parameters E1-E8 are stored in the slot's oscillator waveform and
 * its 7 engine specific parameters (slot_rolePar), so the sequencer's
 * parameter locks, the morph and MIDI reach them without any change of the
 * kit format.
 */
#ifndef SH02_H_
#define SH02_H_

#include <stdint.h>
#include "Shard/sh_engine.h"

#define SH02_FIRST_ENGINE	4
#define SH02_NUM_ENGINES	19		/* SHARD engines (Plaits, MD style, IDM) */
#define SH02_NUM_PARAMS		8		/* engine parameters per voice */

static inline uint8_t sh02_isShard(const uint8_t engine) { return engine >= SH02_FIRST_ENGINE; }

/* engine info of an LXR-02 engine number (must be a SHARD engine) */
const ShEngineInfo* sh02_info(const uint8_t engine);

/* 3 letter name of any engine (stock ones too) */
const char* sh02_engineShortName(const uint8_t engine);
/* up to 8 letter name of any engine */
const char* sh02_engineLongName(const uint8_t engine);

/* the parameter that holds engine parameter k (0-7) of a slot */
uint16_t sh02_param(const uint8_t slot, const uint8_t k);
/* k if par holds an engine parameter of slot, else -1 */
int8_t sh02_paramIndex(const uint8_t slot, const uint16_t par);

/* sound engine side: if par is an engine parameter of a slot that runs a
 * SHARD engine, store it there and return 1 */
uint8_t sh02_setParameter(const uint16_t par, const uint8_t itg, const float flt, const uint8_t isFloatModulator);

/* UI side: display type (enum Datatypes, menu id in the upper byte) of an
 * engine parameter of a SHARD voice in the active kit, 0 if par is none */
uint16_t sh02_dtype(const uint16_t par);
/* list menus of the engine parameters of the active voice (menu ids
 * MENU_SH_E1 .. MENU_SH_E8): number of entries and the label of a value */
uint8_t sh02_menuCount(const uint8_t k);
void sh02_menuLabel(const uint8_t k, const uint8_t value, char* buf3);
/* the names below are of the active voice, or once of this slot (set while a
 * modulation target of another voice is shown; NUM_VOICE_SLOTS = active) */
void sh02_setNameSlot(const uint8_t slot);
uint8_t sh02_nameSlotOrActive(void);
/* the text index (TEXT_SH_E1 + k) of a modulation target of a SHARD voice, 0
 * if par isn't one, 0xFF (MOD_TARGET_HIDDEN) for an unused engine parameter.
 * Sets the name slot. */
uint8_t sh02_modTargetName(const uint16_t par);
/* names of engine parameter k of the active voice (3 and up to 8 letters) */
const char* sh02_paramShortName(const uint8_t k);
const char* sh02_paramLongName(const uint8_t k);

/* defaults of a SHARD engine into the active kit's parameters of a slot
 * (engine parameters, pitch, amp EG, filter); called after the stock
 * defaults have been written */
void sh02_applyDefaults(const uint8_t slot, const uint8_t engine, uint8_t* params);

#endif /* SH02_H_ */
