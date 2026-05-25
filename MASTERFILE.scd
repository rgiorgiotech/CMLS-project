/*
    ============================================================
    COMPUTER MUSIC PROJECT - SUPERCOLLIDER BACKBONE
    ============================================================

    Pipeline:

    MIDI keyboard / virtual piano
        -> SuperCollider MIDI input
        -> Mapping layer
        -> SuperCollider synth
        -> OSC messages to JUCE

    Current version:
        - MIDI notes trigger a placeholder synth.
        - Velocity controls amplitude.
        - MIDI note number controls frequency.
        - SC sends frequency and amplitude to JUCE through OSC.
*/


/*
    ============================================================
    0. CLEANUP
    ============================================================
*/

(
MIDIdef.freeAll;

if(~activeNotes.notNil) {
    ~activeNotes.do({ |synth|
        synth.set(\gate, 0);
    });
    ~activeNotes.clear;
};

"Previous MIDI definitions and notes cleared.".postln;
)


/*
    ============================================================
    1. BOOT SERVER
    ============================================================
*/

(
s.waitForBoot({

    "Server booted.".postln;


    /*
        ========================================================
        2. MIDI SETUP
        ========================================================
    */

    MIDIClient.init;
    MIDIIn.connectAll;

    "MIDI initialized.".postln;
    "Available MIDI sources:".postln;
    MIDIClient.sources.do({ |src, i|
        [i, src].postln;
    });


    /*
        ========================================================
        3. GLOBAL STORAGE
        ========================================================
    */

    ~activeNotes = IdentityDictionary.new;


    /*
        ========================================================
        4. OSC SETUP TO JUCE
        ========================================================

        Assumption:
        JUCE will listen on localhost, port 9001.

        Later, in JUCE:
            receive /note/on
            receive /note/off
            receive /note/freq
            receive /note/amp
    */

    ~juceAddress = NetAddr("127.0.0.1", 9001);

    ~sendNoteOnToJuce = { |note, freq, amp|
        ~juceAddress.sendMsg("/note/on", note, freq, amp);
        ~juceAddress.sendMsg("/note/freq", freq);
        ~juceAddress.sendMsg("/note/amp", amp);
    };

    ~sendNoteOffToJuce = { |note|
        ~juceAddress.sendMsg("/note/off", note);
    };

    "OSC sender to JUCE configured at 127.0.0.1:9001.".postln;


    /*
        ========================================================
        5. SYNTH DEFINITIONS
        ========================================================

        This is a placeholder synth.

        Later your group can replace only this SynthDef with:
            - granular synth
            - cosine synth
            - FM synth
            - noise percussion
            - sample-based synth
    */

    SynthDef(\mainSynth, {
        |out = 0, freq = 440, amp = 0.2, gate = 1|

        var sig;
        var env;

        env = EnvGen.kr(
            Env.adsr(
                attackTime: 0.01,
                decayTime: 0.15,
                sustainLevel: 0.6,
                releaseTime: 0.4
            ),
            gate,
            doneAction: 2
        );

        sig = Saw.ar(freq);

        sig = LPF.ar(sig, 3000);

        sig = sig * env * amp;

        Out.ar(out, sig ! 2);
    }).add;


    /*
        Wait until SynthDef is loaded on the server.
    */

    s.sync;

    "SynthDefs loaded.".postln;


    /*
        ========================================================
        6. SYNTH CONTROL FUNCTIONS
        ========================================================

        These functions are the interface between mapping and sound.

        Mapping calls:
            ~playSynth.value(note, freq, amp);

        It does NOT directly call Synth(...).
    */

    ~playSynth = { |note, freq, amp|

        var synth;

        if(~activeNotes[note].notNil) {
            ~activeNotes[note].set(\gate, 0);
        };

        synth = Synth(\mainSynth, [
            \freq, freq,
            \amp, amp,
            \gate, 1
        ]);

        ~activeNotes[note] = synth;
    };


    ~releaseSynth = { |note|

        if(~activeNotes[note].notNil) {
            ~activeNotes[note].set(\gate, 0);
            ~activeNotes.removeAt(note);
        };
    };


    /*
        ========================================================
        7. MAPPING LAYER
        ========================================================

        This is where musical decisions happen.

        Current mapping:
            MIDI note number -> frequency in Hz
            velocity         -> amplitude in [0, 0.3]

        Later:
            velocity -> grain density
            knob     -> filter cutoff
            note     -> sample position
            pedal    -> freeze / sustain
    */

    ~handleNoteOn = { |note, velocity, channel, source|

        var freq;
        var amp;

        freq = note.midicps;
        amp = velocity.linlin(1, 127, 0.02, 0.30);

        ~playSynth.value(note, freq, amp);
        ~sendNoteOnToJuce.value(note, freq, amp);

        [
            "NOTE ON",
            "note:", note,
            "freq:", freq.round(0.01),
            "velocity:", velocity,
            "amp:", amp.round(0.001),
            "channel:", channel
        ].postln;
    };


    ~handleNoteOff = { |note, velocity, channel, source|

        ~releaseSynth.value(note);
        ~sendNoteOffToJuce.value(note);

        [
            "NOTE OFF",
            "note:", note,
            "channel:", channel
        ].postln;
    };


    /*
        ========================================================
        8. MIDI CALLBACKS
        ========================================================

        MIDIdef receives structured MIDI events.

        For noteOn:
            velocity, note number, channel, source

        For noteOff:
            velocity, note number, channel, source
    */

    MIDIdef.noteOn(\projectNoteOn, {
        |velocity, note, channel, source|

        ~handleNoteOn.value(note, velocity, channel, source);
    });


    MIDIdef.noteOff(\projectNoteOff, {
        |velocity, note, channel, source|

        ~handleNoteOff.value(note, velocity, channel, source);
    });


    /*
        ========================================================
        9. OPTIONAL: CONTROL CHANGE HANDLER
        ========================================================

        This is for knobs/sliders later.

        MIDI CC message:
            ccNumber -> which knob/controller
            value    -> value in [0, 127]
    */

    MIDIdef.cc(\projectCC, {
        |value, ccNumber, channel, source|

        [
            "CONTROL CHANGE",
            "cc:", ccNumber,
            "value:", value,
            "normalized:", (value / 127).round(0.001),
            "channel:", channel
        ].postln;

        /*
            Example future mapping:

            if(ccNumber == 1) {
                ~cutoff = value.linexp(0, 127, 200, 8000);
                ~juceAddress.sendMsg("/effect/cutoff", ~cutoff);
            };
        */
    });


    /*
        ========================================================
        10. READY
        ========================================================
    */

    "============================================================".postln;
    "PROJECT BACKBONE READY.".postln;
    "Play your virtual piano or MIDI keyboard.".postln;
    "Expected result: sound + NOTE ON / NOTE OFF messages.".postln;
    "============================================================".postln;

});
)