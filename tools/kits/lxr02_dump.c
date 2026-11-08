/*
 * lxr02_dump.c - dump the LXR-02 parameter numbers, the modulation target
 * table and the SHARD engine numbers as JSON, for convert_shard.py.
 * Built and linked with the host engine test objects (see make_projects.sh).
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "Parameters.h"
#include "menu/CcNr2Text.h"
#include "Soundengine/DSPAudio/VoiceSlot.h"
#include "Shard/sh02.h"
#include "Preset/InitKit.h"

extern const ModTarg modTargets[];

/* the engine objects need these from the harness main */
float adc_getFaderVolume(uint8_t v) { (void)v; return 1.f; }
void frontPanel_sendData(uint8_t s, uint8_t d1, uint8_t d2) { (void)s; (void)d1; (void)d2; }
uint32_t HAL_RNG_GetRandomNumber(RNG_HandleTypeDef* h) { (void)h; return 0; }

#define P(name) printf("  \"%s\": %d,\n", #name, name)
int main(void)
{
	printf("{\n \"par\": {\n");
	P(PAR_COARSE1); P(PAR_FINE1); P(PAR_VELOA1); P(PAR_VELOD1); P(PAR_VELOD6_OPEN);
	P(PAR_VOL_SLOPE1); P(PAR_FILTER_FREQ_1); P(PAR_RESO_1); P(PAR_FILTER_TYPE_1); P(PAR_FILTER_DRIVE_1);
	P(PAR_VOL1); P(PAR_PAN1); P(PAR_PAN4); P(PAR_DRIVE1); P(PAR_SNARE_DISTORTION); P(PAR_VOICE_DECIMATION1);
	P(PAR_AUDIO_OUT1); P(PAR_VOLUME_MOD_ON_OFF1); P(PAR_TRANS1_VOL); P(PAR_TRANS1_WAVE); P(PAR_TRANS1_FREQ); P(PAR_PWM_1);
	P(PAR_FREQ_LFO1); P(PAR_AMOUNT_LFO1); P(PAR_WAVE_LFO1); P(PAR_VOICE_LFO1); P(PAR_TARGET_LFO1);
	P(PAR_RETRIGGER_LFO1); P(PAR_SYNC_LFO1); P(PAR_OFFSET_LFO1); P(PAR_VEL_DEST_1); P(PAR_VELO_MOD_AMT_1);
	P(PAR_FX_TYPE); P(PAR_FX_ROUTING); P(PAR_FX_DRY_WET); P(PAR_FX_DIST_TYPE); P(PAR_FX_P1);
	P(PAR_OSC_WAVE_DRUM1); P(PAR_OSC_WAVE_DRUM2); P(PAR_OSC_WAVE_DRUM3); P(PAR_OSC_WAVE_SNARE); P(PAR_WAVE1_CYM); P(PAR_WAVE1_HH);
	printf("  \"END_OF_SOUND_PARAMETERS\": %d\n },\n", END_OF_SOUND_PARAMETERS);
	printf(" \"role\": [");
	for(int s=0;s<NUM_VOICE_SLOTS;s++) {
		printf("%s[", s ? ", " : "");
		for(int r=0;r<NUM_SLOT_ROLES;r++) printf("%s%d", r ? ", " : "", slot_rolePar[s][r]);
		printf("]");
	}
	printf("],\n \"engine_param\": [");
	for(int s=0;s<NUM_VOICE_SLOTS;s++) {
		printf("%s[", s ? ", " : "");
		for(int k=0;k<SH02_NUM_PARAMS;k++) printf("%s%d", k ? ", " : "", sh02_param((uint8_t)s,(uint8_t)k));
		printf("]");
	}
	printf("],\n \"sh02_first_engine\": %d, \"num_engines\": %d,\n", SH02_FIRST_ENGINE, NUM_ENGINES);
	printf(" \"init_kit\": [");
	for(int i=0;i<INIT_KIT_SIZE;i++) printf("%s%d", i ? ", " : "", initKit[i]);
	printf("],\n");
	printf(" \"mod_targets\": [");
	const uint8_t n = getNumModTargets();
	for(int i=0;i<n;i++) printf("%s[%d, %d]", i ? ", " : "", modTargets[i].param, voiceFromModTargValue((uint8_t)i));
	printf("]\n}\n");
	return 0;
}
