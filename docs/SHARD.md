# SHARD engines

SHARD-02 adds the 19 drum engines of SHARD (the firmware for the LXR Eurorack module) to the
voice slots of the LXR-02. The engines are the same code as on the module, with the same numbers;
the LXR-02 keeps its own voice structure around them (amp EG, transient, filter, drive, LFO, mix).

For the stock engines, engine changes and loading voices from other kits see
[ENGINES.md](ENGINES.md); everything there applies to the SHARD engines too.

## Engines

| # | Name | Family | OSC page: E1-E4 | FM page: E5-E8 |
|---|---|---|---|---|
| 0-3 | drum, snare, cymbal, hi-hat | LXR-02 | stock engines | |
| 4 | AnalogBD | Plaits | Ton, Dec, AFM, SFM | Drv |
| 5 | SynthBD | Plaits | Ton, Dec, Drt, FMA | FMD |
| 6 | AnalogSD | Plaits | Ton, Dec, Snp | |
| 7 | SynthSD | Plaits | FMA, Dec, Snp | |
| 8 | Hat | Plaits | Ton, Dec, Noi, Typ (808/Rng) | Opn |
| 9 | Modal | Plaits | Str, Bri, Dmp, Exc | |
| 10 | String | Plaits | Str, Bri, Dmp, Exc | |
| 11 | Particle | Plaits | Den, Res, Spr | |
| 12 | TRX-BD | drum computer | Swp, SwD, Dec, Hld | Clk, Drv |
| 13 | TRX-SD | drum computer | Bdy, Snp, Dec, NDc | Swp, NTn |
| 14 | TRX-CP | drum computer | Dec, Spc, Num (1-8), Q | Rnd |
| 15 | TRX-HH | drum computer | Ton, Dec, Mtl, Opn | Res |
| 16 | EFM | FM drum | Rat (ratio list), Mod, MDc, FB | Swp, SwD, Dec, Noi |
| 17 | GND | FM drum | Wav (Sin/Tri/Saw/Sqr/Noi/Imp/S&H), Swp, SwD | |
| 18 | Zap | IDM | Dep, Tim, FB, Dec | Dir (Dwn/Up/Bth) |
| 19 | Crush | IDM | Src (Noi/Sqr/Saw/FM), Rat, Bit, Jit | Dec, Glt |
| 20 | Bytebeat | IDM | Frm (F1-F8), A, B, Rst | |
| 21 | RingClst | IDM | Spr, RM, Ton, Res | Wav (Sqr/Sin) |
| 22 | Comb | IDM | Exc (Imp/Noi/Clk/Saw), FB (±63), Dmp, Bur | Rat, Mx2 |

For Modal and String, `Dmp` sets the decay length: higher is longer. `Opn` (Hat, TRX-HH) is the
open hi-hat setting used by track 7 when the engine runs in slot 6.

The Plaits code is taken from the Mutable Instruments repository (MIT) almost unchanged; only the
sample rate was adjusted. The drum computer and FM drum engines are original code written from
published descriptions of classic drum machines; no third-party code was used for them.

## Selecting an engine

On the device: select a voice, press SHIFT + KIT and choose the engine with the encoder (after the
4 stock engines come the 19 SHARD engines), then press the encoder button. Everything else works
as described in [ENGINES.md](ENGINES.md): the slot starts from the new engine's defaults, and the
engine is saved in the kit.

Via MIDI: NRPN 99 (CC 99 = 0, CC 98 = 99, CC 6 = value), value = slot × 19 + (engine − 4), with
slots 0-5 and engines 4-22. Example for slot 0 → TRX-BD (engine 12, value 8):
`B0 63 00 B0 62 63 B0 06 08`. NRPN 98 still selects the stock engines. Like NRPN 98, only a
received NRPN changes the engine; it is not a kit parameter.

## Pages of a SHARD voice

A SHARD voice uses the pages of the drum voice:

| Page | Parameters |
|---|---|
| OSC | coarse, fine (tuning of the engine), E1, E2, E3, E4 |
| AEG | attack, decay, slope (in slot 6: decay A for track 6, decay B for track 7) |
| MOD | velocity destination, velocity amount, velocity → volume (no pitch envelope: the engines have their own) |
| FM | E5, E6, E7, E8 |
| transient | wave, volume, frequency (added before the filter, as on the drum voice) |
| filter | frequency, resonance, type, drive |
| LFO, MIX | unchanged |

The display shows the engine's own names (`Ton`, `Dec`, ...) for E1-E8. Unused E slots are
empty. Lists (wave, type, ratio, ...) show their entries, on/off switches on/off, bipolar
parameters −63..+63, the rest 0..127.

The engine parameters are stored in the slot's stock parameters: E1 in the oscillator wave
parameter, E2-E8 in the engine-specific parameters the slot already has. So a SHARD kit is an
ordinary kit file, and parameter locks, morph, the LFOs and velocity modulation (as targets with
the engine's names) and MIDI work on E1-E8 like on any other parameter. MIDI numbers of E1-E8
(CC n for parameters 1-127, NRPN n−128 for 128-255):

| Slot | E1 | E2 | E3 | E4 | E5 | E6 | E7 | E8 |
|---|---|---|---|---|---|---|---|---|
| 1 | CC 1 | CC 20 | CC 70 | CC 74 | CC 78 | CC 82 | CC 83 | NRPN 6 |
| 2 | CC 2 | CC 21 | CC 71 | CC 75 | CC 79 | CC 84 | CC 85 | NRPN 7 |
| 3 | CC 3 | CC 22 | CC 72 | CC 76 | CC 80 | CC 86 | CC 87 | NRPN 8 |
| 4 | CC 4 | NRPN 95 | CC 73 | CC 77 | CC 81 | CC 28 | CC 27 | CC 68 |
| 5 | CC 6 | CC 23 | CC 24 | CC 30 | CC 32 | CC 31 | CC 29 | CC 69 |
| 6 | CC 7 | CC 25 | CC 26 | CC 34 | CC 36 | CC 35 | CC 33 | NRPN 96 |

Because the same parameters mean different things for different engines, a step lock or morph
between two kits only makes sense where both kits use the same engine in that slot. Morph already
skips the engine-specific parameters of a slot whose engine differs in the morph kit.

## Not ported from the Eurorack module

The LXR-02 has its own voice structure, so these SHARD features are not part of SHARD-02:

- the CV matrix (the LFOs and velocity modulation take its place) and the accent inputs;
- the per-channel ring modulator and the bit reduction of a channel (`bits`);
- the glitch FX and the FX bus ring modulator.

The SHARD sequencer extensions for the LXR-02 come later.

## Kits

SHARD-02 kits use the OpenElixir-02 kit format (247 parameter bytes followed by the engine block),
with engine numbers 4-22 for the SHARD engines. OpenElixir-02 and the stock firmware load them, but
play every slot with a SHARD engine with its stock engine (and the SHARD settings in its stock
parameters, which usually sounds wrong). Kits of OpenElixir-02 and the stock firmware load and
play in SHARD-02 as before.

### Factory projects 40-49

The 5 kit banks of SHARD 0.5 (128 kits each) are converted into 10 projects of 64 kits:

| Projects | Bank | Contents |
|---|---|---|
| 40, 41 | 00 `SHARD05` | factory bank (MD / PL / ID / HY groups) |
| 42, 43 | 01 `TECHNO` | techno with rumble, atmosphere and pumping kicks |
| 44, 45 | 02 `ELECTRO` | hard Detroit electro, 808-style kicks |
| 46, 47 | 03 `PSY` | dark / forest psytrance, rolling bass |
| 48, 49 | 04 `INDSTRL` | hard industrial |

The first project of each pair has kits 0-63 of the bank, the second kits 64-127, in the same
order. Channel 1-6 of the module is voice slot 1-6, its T7 is track 7.

Copy the `proj40` ... `proj49` folders to the root of the SD card. The projects hold kits only, no
patterns or songs. The easiest way to use them is LOAD + KIT → "Change Project" (below kit 0) →
project 40-49, which loads single kits into the current project. Loading one of the projects
itself keeps the patterns that are in memory.

The conversion keeps engines, engine parameters, tuning, amp EG, filter, drive, sample rate,
volume, pan, outputs, velocity → volume, the FX bus (drive, compressor or delay with its
settings) and the first LFO target of each SHARD LFO. What it can't keep (bits, glitch FX, further LFO targets, accents) is listed per kit in
`conversion_notes.txt`. A converted kit sounds close to the original, a little louder (about
+3 dB, the LXR-02 sums the voices differently), and without the bit reduction and glitch.

### Converting kits

`tools/kits/convert_shard.py` converts SHARD kit banks:

```sh
python3 tools/kits/convert_shard.py <shard.json> <lxr02.json> <out dir> <bank dir> <project nr> [<bank dir> <project nr> ...]
```

`shard.json` is the parameter layout of the SHARD firmware (`shard_render --dump` from the SHARD
repository), `lxr02.json` that of SHARD-02 (`tools/kits/lxr02_dump`). Each bank becomes 2 projects, starting at the given number.
