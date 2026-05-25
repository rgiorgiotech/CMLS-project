/*
    SYNTH DEFINITIONS

    Touchpad layer system:
    - 8 touchpads toggle 8 synth layers.
    - Piano notes trigger all currently enabled layers.
    - Each note stores an array of Synth objects.
*/


// ============================================================
// LAYER 1: SINE
// ============================================================

SynthDef(\layerSine, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(0.01, 0.15, 0.7, 0.5),
        gate,
        doneAction: 2
    );

    sig = SinOsc.ar(freq);
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 2: SAW
// ============================================================

SynthDef(\layerSaw, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(0.02, 0.12, 0.6, 0.45),
        gate,
        doneAction: 2
    );

    sig = Saw.ar(freq);
    sig = LPF.ar(sig, (freq * 4).clip(300, 9000));
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 3: PULSE
// ============================================================

SynthDef(\layerPulse, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(0.01, 0.1, 0.55, 0.35),
        gate,
        doneAction: 2
    );

    sig = Pulse.ar(freq, 0.35);
    sig = LPF.ar(sig, (freq * 3).clip(250, 7500));
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 4: FM BELL
// ============================================================

SynthDef(\layerFM, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env, mod;

    env = EnvGen.kr(
        Env.adsr(0.005, 0.25, 0.35, 0.8),
        gate,
        doneAction: 2
    );

    mod = SinOsc.ar(freq * 2.01, 0, freq * 1.5);
    sig = SinOsc.ar(freq + mod);
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 5: FILTERED NOISE
// ============================================================

SynthDef(\layerNoise, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(0.01, 0.2, 0.25, 0.5),
        gate,
        doneAction: 2
    );

    sig = WhiteNoise.ar;
    sig = BPF.ar(sig, freq * 2, 0.25);
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 6: SUB OSCILLATOR
// ============================================================

SynthDef(\layerSub, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(0.02, 0.12, 0.75, 0.5),
        gate,
        doneAction: 2
    );

    sig = SinOsc.ar(freq / 2);
    sig = LPF.ar(sig, 500);
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 7: GLITCH / GRAIN-LIKE
// ============================================================

SynthDef(\layerGlitch, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env, trig;

    env = EnvGen.kr(
        Env.adsr(0.005, 0.08, 0.35, 0.25),
        gate,
        doneAction: 2
    );

    trig = LFPulse.kr(freq / 16, 0, 0.2);
    sig = Decay2.kr(trig, 0.005, 0.04) * Saw.ar(freq * [1, 1.01]);
    sig = sig.sum * 0.5;
    sig = HPF.ar(sig, 700);
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER 8: PAD / DRONE
// ============================================================

SynthDef(\layerPad, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(0.6, 0.5, 0.65, 1.5),
        gate,
        doneAction: 2
    );

    sig = Mix([
        Saw.ar(freq * 0.995),
        Saw.ar(freq),
        Saw.ar(freq * 1.005)
    ]) / 3;

    sig = LPF.ar(sig, (freq * 2.5).clip(300, 5000));
    sig = sig * env * amp;

    Out.ar(out, sig ! 2);
}).add;


// ============================================================
// LAYER STATE
// ============================================================

~layerSynthDefs = [
    \layerSine,
    \layerSaw,
    \layerPulse,
    \layerFM,
    \layerNoise,
    \layerSub,
    \layerGlitch,
    \layerPad
];

// Default: layer 1 active, all others inactive.
~enabledLayers = [
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    false
];


// ============================================================
// SYNTH INSTANCE STORAGE
// ============================================================

// ~activeNotes is used by note_state.scd.
// ~activeSynths stores actual Synth objects.
// Each note maps to an Array of Synths.

~activeSynths = IdentityDictionary.new;


// ============================================================
// TOGGLE LAYER
// ============================================================

// ============================================================
// SET LAYER STATE
// ============================================================

~setLayerState = { |layerIndex, state|

    var oldState;

    if ((layerIndex >= 0) and: { layerIndex < ~enabledLayers.size }) {

        oldState = ~enabledLayers[layerIndex];

        if (oldState != state) {

            ~enabledLayers[layerIndex] = state;

            [
                "LAYER STATE",
                "index:", layerIndex + 1,
                "name:", ~layerNames[layerIndex],
                "state:", state
            ].postln;

            // Optional semantic OSC to JUCE GUI
            if (~juceAddress.notNil) {
                ~juceAddress.sendMsg(
                    "/juce/button",
                    ("LAYER_" ++ (layerIndex + 1)).asString,
                    state.binaryValue
                );
            };

            // Optional semantic OSC to Processing
            if (~processingAddress.notNil) {
                ~processingAddress.sendMsg(
                    "/visual/state",
                    ("LAYER_" ++ (layerIndex + 1)).asString,
                    state.binaryValue
                );
            };
        };
    };
};


// ============================================================
// PLAY SYNTH LAYERS
// ============================================================

~playSynth = { |note, freq, amp|

    var synthArray;
    var oldestKey;

    // Release previous synths on same note if repeated
    if (~activeSynths[note].notNil) {
        ~activeSynths[note].do({ |synth|
            synth.set(\gate, 0);
        });

        ~activeSynths.removeAt(note);
    };

    // Voice limit
    if (~activeSynths.size >= ~maxVoices) {
        oldestKey = ~activeSynths.keys.asArray.first;

        if (oldestKey.notNil) {
            ~activeSynths[oldestKey].do({ |synth|
                synth.set(\gate, 0);
            });

            ~activeSynths.removeAt(oldestKey);

            if (~forgetNote.notNil) {
                ~forgetNote.value(oldestKey);
            };
        };
    };

    synthArray = Array.new;

    ~enabledLayers.do({ |enabled, i|

        if (enabled) {
            synthArray = synthArray.add(
                Synth(~layerSynthDefs[i], [
                    \freq, freq,
                    \amp, amp * ~layerGains[i],
                    \gate, 1
                ])
            );
        };
    });

    ~activeSynths[note] = synthArray;

    // Safety timer: releases the synth if noteOff never arrives.
    SystemClock.sched(~noteSafetyReleaseTime, {
        if (~activeSynths[note].notNil) {
            ~activeSynths[note].do({ |synth|
                synth.set(\gate, 0);
            });

            ~activeSynths.removeAt(note);

            if (~forgetNote.notNil) {
                ~forgetNote.value(note);
            };

            ("Safety release note: " ++ note).postln;
        };

        nil;
    });
};


// ============================================================
// RELEASE SYNTH
// ============================================================

~releaseSynth = { |note|

    if (~activeSynths[note].notNil) {
        ~activeSynths[note].do({ |synth|
            synth.set(\gate, 0);
        });

        ~activeSynths.removeAt(note);
    };
};


// ============================================================
// RELEASE ALL SYNTHS
// ============================================================

~releaseAllSynths = {

    if (~activeSynths.notNil) {
        ~activeSynths.do({ |synthArray|
            synthArray.do({ |synth|
                synth.set(\gate, 0);
            });
        });

        ~activeSynths.clear;
    };
};


"Synth layers loaded.".postln;