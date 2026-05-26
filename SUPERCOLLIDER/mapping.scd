/*
    MAPPING LAYER

    Purpose:
    - Convert semantic note information into project actions.
    - Trigger local SC synths.
    - Send note information to JUCE.
    - Send note information to Processing.

    Important:
    Raw MIDI is NOT handled here.
    Raw MIDI enters through midi_input.scd.
*/


// ============================================================
// OSC: NOTES TO JUCE
// ============================================================

~sendNoteOnToJuce = { |info|

    ~juceAddress.sendMsg(
        "/note/on",
        info[\note],
        info[\freq],
        info[\amp],
        info[\velocityNorm],
        info[\polyphony],
        info[\register].asString,
        info[\isLegato].binaryValue,
        info[\isRepeated].binaryValue
    );

    // Keep old simple messages for compatibility
    ~juceAddress.sendMsg("/note/freq", info[\freq]);
    ~juceAddress.sendMsg("/note/amp", info[\amp]);
};


~sendNoteOffToJuce = { |info|

    ~juceAddress.sendMsg(
        "/note/off",
        info[\note],
        info[\duration],
        info[\polyphonyAfterRelease]
    );
};


// ============================================================
// OSC: NOTES TO PROCESSING
// ============================================================

~sendNoteOnToProcessing = { |info|

    ~processingAddress.sendMsg(
        "/visual/note/on",
        info[\note],
        info[\velocityNorm],
        info[\polyphony],
        info[\register].asString,
        info[\isLegato].binaryValue,
        info[\isRepeated].binaryValue
    );

    // Keep old simple message for compatibility
    ~processingAddress.sendMsg(
        "/visual/note",
        info[\note],
        info[\velocityNorm]
    );
};


~sendNoteOffToProcessing = { |info|

    ~processingAddress.sendMsg(
        "/visual/note/off",
        info[\note],
        info[\duration],
        info[\polyphonyAfterRelease]
    );
};


// ============================================================
// NOTE ON HANDLER
// ============================================================

~handleNoteOn = { |note, velocity, channel, source|

    var info;

    if (note <= ~highestAllowedNote) {

        info = ~registerNoteOn.value(note, velocity, channel, source);

        ~playSynth.value(
            info[\note],
            info[\freq],
            info[\amp]
        );

        ~sendNoteOnToJuce.value(info);
        ~sendNoteOnToProcessing.value(info);

        [
            "NOTE ON",
            "note:", info[\note],
            "freq:", info[\freq].round(0.01),
            "vel:", info[\velocity],
            "amp:", info[\amp].round(0.001),
            "velNorm:", info[\velocityNorm].round(0.001),
            "poly:", info[\polyphony],
            "register:", info[\register],
            "legato:", info[\isLegato],
            "repeated:", info[\isRepeated]
        ].postln;
    };
};

// ============================================================
// NOTE OFF HANDLER
// ============================================================

~handleNoteOff = { |note, velocity, channel, source|

    var info;

    info = ~registerNoteOff.value(note, velocity, channel, source);

    ~releaseSynth.value(note);

    ~sendNoteOffToJuce.value(info);
    ~sendNoteOffToProcessing.value(info);

    [
        "NOTE OFF",
        "note:", info[\note],
        "duration:", info[\duration].round(0.001),
        "polyAfter:", info[\polyphonyAfterRelease]
    ].postln;
};


// ============================================================
// CC HANDLER
// Existing simple CC forwarding.
// Not part of this note-only improvement.
// ============================================================

~handleCC = { |value, ccNumber, channel, source|

    var normalized;
    var padIndex;
    var isPressed;

    normalized = value / 127.0;

    // ------------------------------------------------------------
    // TOUCHPAD CC HANDLING
    // Direct physical-state mapping:
    // red/touched  -> layer ON
    // dark/release -> layer OFF
    // ------------------------------------------------------------

    padIndex = ~touchpadCCs.indexOf(ccNumber);

    if (padIndex.notNil) {

        isPressed = normalized > ~touchpadThreshold;

        ~touchpadPressed[padIndex] = isPressed;

        ~setLayerState.value(padIndex, isPressed);

        [
            "PAD CC",
            "cc:", ccNumber,
            "pad:", padIndex + 1,
            "val:", normalized.round(0.001),
            "pressed:", isPressed
        ].postln;

        ^nil;
    };


    // ------------------------------------------------------------
    // NORMAL CC HANDLING
    // ------------------------------------------------------------

    ~juceAddress.sendMsg("/cc", ccNumber, normalized);

    ["CC", ccNumber, "val:", normalized.round(0.001)].postln;
};

"Mapping loaded.".postln;