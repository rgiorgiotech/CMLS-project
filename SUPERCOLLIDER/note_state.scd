/*
    NOTE STATE LAYER

    Purpose:
    - Stores musical note state.
    - Computes note duration.
    - Computes velocity normalization.
    - Computes polyphony count.
    - Detects repeated notes.
    - Detects legato notes.
    - Classifies note register.

    Important:
    This file does NOT create Synths.
    This file does NOT send OSC.
    This file does NOT know about JUCE or Processing.
*/


~activeNotes = IdentityDictionary.new;


// ------------------------------------------------------------
// Utility: MIDI note register classification
// ------------------------------------------------------------

~getNoteRegister = { |note|

    if (note <= ~lowRegisterMax) {
        \low;
    } {
        if (note <= ~midRegisterMax) {
            \mid;
        } {
            \high;
        };
    };
};


// ------------------------------------------------------------
// Utility: velocity -> normalized value
// ------------------------------------------------------------

~velocityToNorm = { |velocity|
    velocity.clip(0, 127) / 127.0;
};


// ------------------------------------------------------------
// Utility: velocity -> amplitude
// ------------------------------------------------------------

~velocityToAmp = { |velocity|
    velocity.clip(1, 127).linlin(
        1,
        127,
        ~noteVelocityAmpMin,
        ~noteVelocityAmpMax
    );
};


// ------------------------------------------------------------
// Register note on
// ------------------------------------------------------------

~registerNoteOn = { |note, velocity, channel, source|

    var now;
    var freq;
    var amp;
    var velocityNorm;
    var wasAlreadyActive;
    var polyphonyBefore;
    var polyphonyAfter;
    var isLegato;
    var register;
    var info;

    now = Main.elapsedTime;

    freq = note.midicps;
    amp = ~velocityToAmp.value(velocity);
    velocityNorm = ~velocityToNorm.value(velocity);

    wasAlreadyActive = ~activeNotes[note].notNil;
    polyphonyBefore = ~activeNotes.size;

    if (wasAlreadyActive) {
        polyphonyAfter = polyphonyBefore;
    } {
        polyphonyAfter = polyphonyBefore + 1;
    };

    isLegato = polyphonyBefore > 0;
    register = ~getNoteRegister.value(note);

    info = (
        note: note,
        freq: freq,
        velocity: velocity,
        velocityNorm: velocityNorm,
        amp: amp,
        channel: channel,
        source: source,
        startTime: now,
        register: register,
        isRepeated: wasAlreadyActive,
        isLegato: isLegato,
        polyphony: polyphonyAfter
    );

    ~activeNotes[note] = info;

    info;
};


// ------------------------------------------------------------
// Register note off
// ------------------------------------------------------------

~registerNoteOff = { |note, velocity, channel, source|

    var now;
    var info;
    var duration;
    var releaseInfo;

    now = Main.elapsedTime;
    info = ~activeNotes[note];

    if (info.notNil) {

        duration = now - info[\startTime];

        releaseInfo = info.copy;
        releaseInfo[\duration] = duration;
        releaseInfo[\releaseVelocity] = velocity;
        releaseInfo[\polyphonyAfterRelease] = (~activeNotes.size - 1).max(0);

        ~activeNotes.removeAt(note);

        releaseInfo;

    } {
        (
            note: note,
            velocity: velocity,
            channel: channel,
            source: source,
            duration: 0.0,
            releaseVelocity: velocity,
            polyphonyAfterRelease: ~activeNotes.size,
            missingNoteOn: true
        );
    };
};


// ------------------------------------------------------------
// Force-remove note state, used by cleanup/safety release
// ------------------------------------------------------------

~forgetNote = { |note|
    if (~activeNotes[note].notNil) {
        ~activeNotes.removeAt(note);
    };
};


// ------------------------------------------------------------
// Clear all note state
// ------------------------------------------------------------

~clearNoteState = {
    if (~activeNotes.notNil) {
        ~activeNotes.clear;
    };
};


"Note state layer loaded.".postln;