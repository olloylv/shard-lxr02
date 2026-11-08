# SHARD-02

SHARD-02 is custom firmware for the Erica Synths LXR-02 drum machine that adds the 19 drum engines
of [SHARD](https://github.com/olloylv/shard-firmware) (the SHARD firmware for the LXR Eurorack
module) to the voice slots. It is based on
[OpenElixir-02](https://github.com/tunefish777/openelixir-02) 1.2 by tunefish777, which in turn is
based on the open-source [LXR V2 firmware](https://github.com/erica-synths/lxr-desktop-app) by
Erica Synths. Everything OpenElixir-02 does is kept; the SHARD additions are listed first.

> SHARD-02 is not official Erica Synths firmware and is not affiliated with or approved by
> Erica Synths or by the OpenElixir-02 author. You install it at your own risk.

## SHARD additions

**19 more voice engines.** Besides drum, snare, cymbal and hi-hat, each of the 6 voice slots can run
one of the SHARD engines: 8 from Mutable Instruments Plaits (analog/synth bass and snare drums,
hi-hat, modal, string, particle), 6 in the style of drum computers and FM drums (TRX-BD, TRX-SD,
TRX-CP, TRX-HH, EFM, GND) and 5 IDM/glitch engines (Zap, Crush, Bytebeat, RingClst, Comb). They are
chosen like the other engines: select a voice and press SHIFT + KIT, or send NRPN 99.

**Same workflow as the stock engines.** A SHARD voice keeps the drum voice's pages: tuning, amp EG,
transient, filter, drive, LFO and mix work as usual, and the engine's own parameters (up to 8) are on
the OSC and FM pages. They are ordinary kit parameters, so parameter locks, morph, the LFOs,
velocity modulation and MIDI CC/NRPN work on them, and the kit file format is unchanged.

**Factory kits.** The 640 kits of the SHARD 0.5 banks, converted for the LXR-02, as projects 40-49.

Details: [docs/SHARD.md](docs/SHARD.md). Changes: [CHANGELOG.md](CHANGELOG.md).

## OpenElixir-02

The rest of this README describes OpenElixir-02, the base of SHARD-02. It applies unchanged.

### Goals

- **Sound and timing come first.** Nothing has a higher priority than uninterrupted audio and a
  steady sequencer: menus, the display, the SD card and MIDI all wait for them.
- **More functionality, fully backwards compatible.** OpenElixir-02 extends what is already there.
  Kits, patterns and projects from the stock firmware load and play as before, the MIDI
  implementation is unchanged, and the update works with the stock bootloader.
- **The stock sound stays available.** Every change to the core sound of the unit comes with a
  switch in the config menu, so the stock sound is always one setting away.

### Features

**Kit management.** Load full kits and single voices from any project:
- Hold LOAD and press step button 1-7 (Drum 1 ... OP HH) to load just that voice from any kit of
  any project into the current kit.
- "Change Project" below kit 0 in the load and save kit menus loads kits from other projects and
  saves kits into them.

**Freely assignable voice engines.** Each of the 6 voice slots can run any engine: drum, snare,
cymbal or hi-hat. Select a voice and press SHIFT + KIT (or send NRPN 98). The engines are saved in
the kit; kits from the stock firmware load with the stock engines.

Details for both: [docs/ENGINES.md](docs/ENGINES.md).

**Limiter.** Voices and FX are summed in 32 bit, followed by a limiter on each output, so loud kits
no longer hard-clip. Levels that didn't clip before are unchanged. Config menu "lim" (2nd page)
switches it off for exactly the stock mix.

**DJ filter.** A filter on the final mix of all outputs, on the performance page: `dj` turned left of
the center is a low-pass (down to 40 Hz), right of it a high-pass (up to 8 kHz), around the center it
is off; `res` sets the resonance. It isn't saved and starts off; while off, the sound is unchanged.

**Uninterrupted playback.** Turning knobs, working in the menus, loading and saving kits (also
from and to other projects on the SD card) and saving projects don't interrupt the audio or the
sequencer while it plays.

**Under the hood.** About 10 % less CPU load, many stock firmware bugs fixed (MIDI, sequencer, SD
card, memory corruption; most found in the firmware review by jhw in
[erica-synths/lxr-desktop-app#3](https://github.com/erica-synths/lxr-desktop-app/issues/3)), and a
project save that can't lose the saved project. Stock issues that are kept on purpose:
[docs/KNOWN_ISSUES.md](docs/KNOWN_ISSUES.md). The changes of each version are in the
[release notes](https://github.com/tunefish777/openelixir-02/releases).

## Building and installing

```sh
cmake --preset release      # add -DARM_TOOLCHAIN_DIR=<toolchain>/bin if arm-none-eabi-gcc isn't on PATH
cmake --build --preset release
```

This creates `build/release/LXR_SHARD-02_V<version>.img`. Copy it to the SD card and install it
with the LXR-02's built-in updater (hold the encoder while switching on). The `LXR` prefix in the file
name is required by the updater.

The GitHub Actions workflow inherited from OpenElixir-02 builds every push; the firmware files are
attached to each run in the repository's Actions tab, and the run summary shows the memory usage
compared with the stock firmware.

The kit converter is in [tools/kits](tools/kits).

Requirements, build options, the memory map, troubleshooting and development details are in the
original firmware's README: **[README_LXR.md](README_LXR.md)**.

## Licence

GPL-3.0-only, with the additional terms in [NOTICE.md](NOTICE.md); see [LICENSE](LICENSE). The SHARD
engines include code from Mutable Instruments Plaits and stmlib by Emilie Gillet (MIT licence, in
`Core/ThirdParty`). "Erica
Synths", "LXR" and "LXR-02" are trademarks of Erica Synths, and the LXR hardware is their property.
