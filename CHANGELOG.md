# Changelog

SHARD-02 versions. The changes of the OpenElixir-02 base are in its
[release notes](https://github.com/tunefish777/openelixir-02/releases).

## 0.5 — test build

First version, based on OpenElixir-02 1.2.

- 19 SHARD engines (engines 4-22) for the 6 voice slots, the same engines and numbers as SHARD 0.5
  on the Eurorack module: 8 Plaits engines, TRX-BD/SD/CP/HH, EFM, GND, Zap, Crush, Bytebeat,
  RingClst, Comb. Selected with SHIFT + KIT or NRPN 99.
- Up to 8 engine parameters (E1-E8) on the OSC and FM pages, stored in the slot's stock
  parameters: parameter locks, morph, LFO and velocity targets and MIDI work on them, the kit
  format is unchanged.
- The drum voice's amp EG, transient, filter, drive, LFO and mix around every SHARD engine.
- Factory projects 40-49: the 640 kits of the SHARD 0.5 banks, converted.
- Kit converter (`tools/kits`) and host engine test (`test/engine`) for the SHARD engines.
- Lighter compressor gain computer (float instead of double math, same sound), so heavy kits with
  the compressor have more CPU headroom.
- Firmware file name `LXR_SHARD-02_V0.5.img`.
