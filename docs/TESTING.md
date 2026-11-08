# Testing SHARD-02 0.5 on an LXR-02

SHARD-02 0.5 has been tested on a host build of the sound engine (all 23 engines in all 6 slots,
kit loading, renders of the converted kits), not yet on the hardware. These are the things to check
on a real LXR-02.

## Before you start

1. Back up the whole SD card, so going back to your own setup is trivial.
2. Copy `LXR_SHARD-02_V0.5.img` to the top folder of the card and delete any older `LXR*.img`
   files there. Copy the folders `proj40` ... `proj49` to the top folder too (they replace nothing
   unless you already have projects 40-49).
3. Put the card in the LXR-02, hold down the encoder button and switch it on. Keep holding the
   button until "LXR Updater" appears, then wait for "Update OK!"; the unit restarts by itself.
   To go back, install the stock or OpenElixir-02 firmware the same way.

## Checklist

**Start and old material**
- [ ] The unit starts, the display and the menus work.
- [ ] An existing project of yours loads and plays exactly as before (stock engines, levels,
      patterns, songs).
- [ ] Saving a project and loading it again works.

**Engines**
- [ ] SHIFT + KIT lists 23 engines; each SHARD engine (4-22) can be selected in each slot and
      makes a sound when the track is triggered.
- [ ] The OSC page shows coarse, fine and the engine's E1-E4 names, the FM page E5-E8; turning
      them changes the sound; lists (e.g. Typ, Wav, Src) show their entries.
- [ ] Slot 6 with a hat engine (Hat, TRX-HH): track 6 closed, track 7 open, each cuts the other.
- [ ] Switching a slot back to a stock engine restores the stock pages.

**Kits and modulation**
- [ ] LOAD + KIT → "Change Project" → project 40-49 loads converted kits; they sound like
      drum kits, not like noise or silence.
- [ ] A kit with SHARD engines saves and loads again with the same engines and settings.
- [ ] Parameter locks on E1-E8 play back; morph between two kits with the same engines is smooth.
- [ ] An LFO with a SHARD parameter as target, and velocity modulation, work.
- [ ] MIDI: NRPN 99 changes the engine; the CCs of E1-E8 (table in [SHARD.md](SHARD.md)) change
      the parameters.

**CPU and audio**
- [ ] Play the heaviest kits with the sequencer running and turn knobs: no dropouts, clicks or
      slowdowns of the sequencer. Good candidates are kits with 4 or more Plaits engines
      (Modal, String, Particle, SynthBD) and the compressor, e.g. the PL... kits in project 40.
- [ ] Six slots with Modal or String at once, with delay or compressor: does it still play
      cleanly?
- [ ] No noise or DC on the outputs with all voices silent.

## What to report

For anything that goes wrong: firmware version, project and kit number (or a saved kit file),
what you did, and what you heard or saw. A phone recording helps for sound problems. Kits that
sound clearly wrong compared with the SHARD Eurorack originals are useful too; the per-kit list
of features that couldn't be converted is in `conversion_notes.txt`.
