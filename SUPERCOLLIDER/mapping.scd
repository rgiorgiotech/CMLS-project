/*
    MAPPING LAYER
*/

~sendNoteOnToJuce = { |note, freq, amp|
    ~juceAddress.sendMsg("/note/on", note, freq, amp);
    ~juceAddress.sendMsg("/note/freq", freq);
    ~juceAddress.sendMsg("/note/amp", amp);
};

~sendNoteOffToJuce = { |note|
    ~juceAddress.sendMsg("/note/off", note);
};

~sendNoteToProcessing = { |note, velocityNorm|
    ~processingAddress.sendMsg("/visual/note", note, velocityNorm);
};

~handleNoteOn = { |note, velocity, channel, source|

    // Ignora note oltre il MIDI 96 (troppo alte, causano loop)
    if (note <= 96) {
        var freq, amp, velocityNorm;

        freq         = note.midicps;
        amp          = velocity.linlin(1, 127, 0.02, 0.12);
        velocityNorm = velocity / 127;

        ~playSynth.value(note, freq, amp);
        ~sendNoteOnToJuce.value(note, freq, amp);
        ~sendNoteToProcessing.value(note, velocityNorm);

        ["NOTE ON", "note:", note, "freq:", freq.round(0.01),
         "vel:", velocity, "amp:", amp.round(0.001)].postln;
    };
};

~handleNoteOff = { |note, velocity, channel, source|
    ~releaseSynth.value(note);
    ~sendNoteOffToJuce.value(note);
    ["NOTE OFF", "note:", note].postln;
};

// Unica definizione di handleCC — manda i CC a JUCE via OSC
~handleCC = { |value, ccNumber, channel, source|
    var normalized = value / 127.0;
    ~juceAddress.sendMsg("/cc", ccNumber, normalized);
    ["CC", ccNumber, "val:", normalized.round(0.001)].postln;
};

"Mapping loaded.".postln;
