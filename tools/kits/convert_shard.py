#!/usr/bin/env python3
"""Convert SHARD Eurorack kit banks into LXR-02 projects.

    convert_shard.py <shard.json> <lxr02.json> <out dir> <bank dir> <project nr> [<bank dir> <project nr> ...]

shard.json  : `shard_render --dump` of the SHARD Eurorack firmware (parameter layout, engines)
lxr02.json  : output of tools/kits/lxr02_dump (LXR-02 parameter numbers, mod targets, init kit)
bank dir    : a SHARD bank folder (shard/prjNN with 128 .snd and prj.nfo)
project nr  : first LXR-02 project; a bank of 128 kits fills two projects of 64 kits
              (project nr and project nr + 1), named "<BANK> A" / "<BANK> B"

Per channel the engine, its parameters, pitch, amp EG, filter, volume, pan, drive,
sample rate, output, velocity -> volume and the LFO are converted. SHARD features the
LXR-02 doesn't have are left out: bit crusher, ring modulator, CV matrix slots other
than the first LFO slot of a channel, glitch FX (-> off), morph settings.
"""
import json
import os
import sys

SH = json.load(open(sys.argv[1]))
LX = json.load(open(sys.argv[2]))
OUT = sys.argv[3]
L = SH["local"]
SP = SH["par"]
XP = LX["par"]
STRIDE = SH["stride"]
CH_BASE = SH["par_ch_base"]
NSOUND = XP["END_OF_SOUND_PARAMETERS"]
ROLE = LX["role"]
EPAR = LX["engine_param"]
FIRST = LX["sh02_first_engine"]
OSC_WAVE = [XP[k] for k in ("PAR_OSC_WAVE_DRUM1", "PAR_OSC_WAVE_DRUM2", "PAR_OSC_WAVE_DRUM3",
                            "PAR_OSC_WAVE_SNARE", "PAR_WAVE1_CYM", "PAR_WAVE1_HH")]
PAN = [XP["PAR_PAN1"], XP["PAR_PAN1"] + 1, XP["PAR_PAN1"] + 2, XP["PAR_PAN4"], XP["PAR_PAN4"] + 1, XP["PAR_PAN4"] + 2]
SIGNATURE = b"OE02"

# SHARD LXR classics (engines 0-3): their parameters -> LXR-02 parameters of the slot
# ("osc" = oscillator waveform, "pwm", "t*" = transient, r0..r6 = roles, None = dropped)
CLASSIC = {
    0: ["osc", "pwm", "r1", "r3", "r2", "r4", "r5", "r0", "r6", "twave", "tvol", "tfreq"],              # Drm
    1: ["osc", None, "r5", None, None, "r1", "r3", "r2", "r4", "pwm", "twave", "tvol", "tfreq"],         # Snr
    2: ["osc", "pwm", "r5", "r2", "r4", "r3", "r0", "r1", "twave", "tvol", "tfreq"],                     # Cym
    3: ["osc", "pwm", "r5", "r2", "r4", "r3", "r0", "r1", "twave", "tvol", "tfreq"],                     # Hat
}
# SHARD filter types off LP HP BP UBP Nch Pk LP2 -> LXR-02 LP HP BP UBP Nch Pek LP2 off
FILTER = [7, 0, 1, 2, 3, 4, 5, 6]
# SHARD FX off drv glt cmp del rng -> LXR-02 off drv rng cmp del (glitch, SHARD ring: off;
# the SHARD 0.5 ring modulator has other parameters than the stock one)
FX = {0: 0, 1: 1, 2: 0, 3: 3, 4: 4, 5: 0}

# mod targets: (param, voice 1-6) -> index
TARGET = {}
for i, (par, voice) in enumerate(LX["mod_targets"]):
    TARGET.setdefault((par, voice), i)


def slot_par(slot, name):
    """LXR-02 parameter of a voice slot by strip function."""
    p = XP
    return {
        "coarse": p["PAR_COARSE1"] + 2 * slot, "fine": p["PAR_FINE1"] + 2 * slot,
        "attack": p["PAR_VELOA1"] + 2 * slot, "decay": p["PAR_VELOD1"] + 2 * slot,
        "slope": p["PAR_VOL_SLOPE1"] + slot, "freq": p["PAR_FILTER_FREQ_1"] + slot,
        "reso": p["PAR_RESO_1"] + slot, "ftype": p["PAR_FILTER_TYPE_1"] + slot,
        "fdrive": p["PAR_FILTER_DRIVE_1"] + slot, "vol": p["PAR_VOL1"] + slot, "pan": PAN[slot],
        "drive": p["PAR_DRIVE1"] + slot, "srate": p["PAR_VOICE_DECIMATION1"] + slot,
        "out": p["PAR_AUDIO_OUT1"] + slot, "velvol": p["PAR_VOLUME_MOD_ON_OFF1"] + slot,
        "tvol": p["PAR_TRANS1_VOL"] + slot, "twave": p["PAR_TRANS1_WAVE"] + slot,
        "tfreq": p["PAR_TRANS1_FREQ"] + slot, "pwm": p["PAR_PWM_1"] + slot, "osc": OSC_WAVE[slot],
        "lfofreq": p["PAR_FREQ_LFO1"] + slot, "lfoamt": p["PAR_AMOUNT_LFO1"] + slot,
        "lfowave": p["PAR_WAVE_LFO1"] + slot, "lfovoice": p["PAR_VOICE_LFO1"] + slot,
        "lfotarget": p["PAR_TARGET_LFO1"] + slot, "lforetrig": p["PAR_RETRIGGER_LFO1"] + slot,
        "lfosync": p["PAR_SYNC_LFO1"] + slot, "lfooffset": p["PAR_OFFSET_LFO1"] + slot,
        "veldest": p["PAR_VEL_DEST_1"] + slot,
    }[name]


def classic_par(slot, name):
    if name[0] == "r":
        return ROLE[slot][int(name[1])]
    return slot_par(slot, name)


# SHARD channel strip locals -> LXR-02 strip function (for the LFO target)
STRIP_TARGET = {
    "ATTACK": "attack", "DECAY": "decay", "SLOPE": "slope", "FILT_FREQ": "freq", "FILT_RESO": "reso",
    "FILT_DRIVE": "fdrive", "VOL": "vol", "PAN": "pan", "SRATE": "srate", "DRIVE": "drive",
    "LFO_FREQ": "lfofreq",
}
LOCAL_NAME = {v: k for k, v in L.items()}


def target_param(engine, dest):
    """LXR-02 (param, voice) of a SHARD mod destination, or None."""
    base = SH["mod_ch_base"]
    if dest < base:
        return None
    if dest >= SH["mod_ext_base"]:
        return None  # ring modulator
    ch, t = divmod(dest - base, SH["mod_targets_per_ch"])
    local = SH["mod_target_local"][t]
    if local == 255:
        return (slot_par(ch, "coarse"), ch + 1)
    name = LOCAL_NAME.get(local, "")
    if L["E0"] <= local < L["E0"] + 29:
        k = local - L["E0"]
        e = engine[ch]
        if e >= FIRST:
            return (EPAR[ch][k], ch + 1) if k < len(EPAR[ch]) else None
        m = CLASSIC[e]
        return (classic_par(ch, m[k]), ch + 1) if k < len(m) and m[k] else None
    if name in STRIP_TARGET:
        return (slot_par(ch, STRIP_TARGET[name]), ch + 1)
    return None


def cv_slot(i):
    if i < 18:
        return SP["CV_SRC_1"] + i, SP["CV_MOD_AMT_1"] + i, SP["CV_DEST_1"] + i
    return SP["CV2_SRC_1"] + i - 18, SP["CV2_AMT_1"] + i - 18, SP["CV2_DEST_1"] + i - 18


def convert(kit):
    name = kit[:8]
    s = kit[8:]
    out = bytearray(LX["init_kit"][8:8 + NSOUND] + [0] * max(0, NSOUND - (len(LX["init_kit"]) - 8)))
    engine = []
    notes = []
    for ch in range(6):
        b = CH_BASE + ch * STRIDE
        loc = lambda n: s[b + (L["E0"] + int(n[1:]) if n[0] == "E" and n[1:].isdigit() else L[n])]
        e = loc("ENGINE")
        engine.append(e)
        if e >= FIRST:
            for k in range(len(EPAR[ch])):
                out[EPAR[ch][k]] = loc("E%d" % k)
        else:
            for k, m in enumerate(CLASSIC[e]):
                if m:
                    out[classic_par(ch, m)] = loc("E%d" % k)
        out[slot_par(ch, "coarse")] = loc("TUNE")
        out[slot_par(ch, "fine")] = loc("FINE")
        own_env = loc("AEG_MODE") == 1
        out[slot_par(ch, "attack")] = 0 if own_env else loc("ATTACK")
        out[slot_par(ch, "decay")] = 127 if own_env else loc("DECAY")
        if ch == 5:
            out[XP["PAR_VELOD6_OPEN"]] = 127 if own_env else loc("DECAY_ALT")
        out[slot_par(ch, "slope")] = loc("SLOPE")
        out[slot_par(ch, "freq")] = loc("FILT_FREQ")
        out[slot_par(ch, "reso")] = loc("FILT_RESO")
        out[slot_par(ch, "ftype")] = FILTER[loc("FILT_TYPE") & 7]
        out[slot_par(ch, "fdrive")] = loc("FILT_DRIVE")
        out[slot_par(ch, "vol")] = loc("VOL")
        out[slot_par(ch, "pan")] = loc("PAN")
        out[slot_par(ch, "drive")] = loc("DRIVE")
        out[slot_par(ch, "srate")] = loc("SRATE")
        out[slot_par(ch, "out")] = min(loc("OUT"), 6)
        out[slot_par(ch, "velvol")] = 1 if loc("VELVOL") else 0
        if e >= FIRST:
            out[slot_par(ch, "tvol")] = 0  # no click layer
        out[slot_par(ch, "lfofreq")] = loc("LFO_FREQ")
        out[slot_par(ch, "lfooffset")] = loc("LFO_OFFSET")
        out[slot_par(ch, "lfowave")] = min(loc("LFO_WAVE"), 7)
        out[slot_par(ch, "lforetrig")] = min(loc("LFO_RETRIG"), 6)
        out[slot_par(ch, "lfosync")] = 0
        out[slot_par(ch, "lfovoice")] = ch + 1
        out[slot_par(ch, "lfotarget")] = 0
        out[slot_par(ch, "lfoamt")] = 0
        out[slot_par(ch, "veldest")] = 0
        if loc("BITS") > 0:
            notes.append("ch%d bits" % (ch + 1))
        if loc("RING_AMT"):
            notes.append("ch%d ring" % (ch + 1))
    # the first CV slot of each channel's LFO (sources 5..10 = LFO 1..6) -> the LXR-02 LFO
    used = set()
    for i in range(36):
        sp, ap, dp = cv_slot(i)
        src, amt, dest = s[sp], s[ap], s[dp]
        if amt == 0 or dest == 0:
            continue
        if 11 <= src <= 16:
            # accent n -> velocity modulation of voice n (it can only target its own voice)
            v = src - 11
            t = target_param(engine, dest)
            idx = TARGET.get(t) if t else None
            if idx is None or t[1] != v + 1 or ("vel", v) in used:
                notes.append("accent%d slot %d" % (v + 1, i))
                continue
            used.add(("vel", v))
            out[slot_par(v, "veldest")] = idx
            out[XP["PAR_VELO_MOD_AMT_1"] + v] = min(127, round(amt * 127 / 100))
            continue
        if not (5 <= src <= 10):
            notes.append("cv%d slot %d" % (src + 1, i))
            continue
        lfo = src - 5
        t = target_param(engine, dest)
        idx = TARGET.get(t) if t else None
        if lfo in used or idx is None:
            notes.append("lfo%d slot %d" % (lfo + 1, i))
            continue
        used.add(lfo)
        out[slot_par(lfo, "lfovoice")] = t[1]
        out[slot_par(lfo, "lfotarget")] = idx
        out[slot_par(lfo, "lfoamt")] = min(127, round(amt * 127 / 100))
    # FX bus
    fx = s[SP["FX_TYPE"]]
    out[XP["PAR_FX_TYPE"]] = FX.get(fx, 0)
    if fx == 2:
        notes.append("glitch fx")
    elif fx == 5:
        notes.append("ring fx")
    # routing, dry/wet, dist type, P1-P4, ratio, wave, reverb type, delay type: same order on both
    for k in range(1, 12):
        out[XP["PAR_FX_TYPE"] + k] = s[SP["FX_TYPE"] + k]
    block = SIGNATURE + bytes([6] + [e for e in engine])
    return bytes(name) + bytes(out) + block, notes


def safe_name(name):
    n = "".join(c if (c.isalnum() or c in "-_") else "_" for c in name.decode("latin-1").rstrip("\x00 "))
    return n[:5] or "kit"


def write_project(folder, title, kits):
    os.makedirs(folder, exist_ok=True)
    with open(os.path.join(folder, "prj.nfo"), "wb") as f:
        f.write(title.encode()[:8].ljust(8, b" "))
    for i, (data, nm) in enumerate(kits):
        with open(os.path.join(folder, "%02d-%s.snd" % (i, safe_name(nm))), "wb") as f:
            f.write(data)


def main():
    args = sys.argv[4:]
    report = []
    for j in range(0, len(args), 2):
        bank, proj = args[j], int(args[j + 1])
        title = open(os.path.join(bank, "prj.nfo"), "rb").read()[:8].decode("latin-1").strip() or "SHARD"
        # kit number = the number in front of the name ("7-..." and "100-..." sort wrongly as text)
        files = sorted((f for f in os.listdir(bank) if f.endswith(".snd")), key=lambda f: int(f.split("-")[0]))
        kits = []
        for f in files:
            raw = open(os.path.join(bank, f), "rb").read()
            data, notes = convert(raw)
            kits.append((data, raw[:8]))
            if notes:
                report.append("%s/%s: %s" % (os.path.basename(bank), f, ", ".join(notes)))
        for half in range(2):
            part = kits[half * 64:(half + 1) * 64]
            if not part:
                continue
            write_project(os.path.join(OUT, "proj%02d" % (proj + half)), "%s %s" % (title[:6], "AB"[half]), part)
    with open(os.path.join(OUT, "conversion_notes.txt"), "w") as f:
        f.write("SHARD features the LXR-02 version doesn't have, per kit:\n")
        f.write("\n".join(report) + "\n")
    print("%d notes" % len(report))


if __name__ == "__main__":
    main()
