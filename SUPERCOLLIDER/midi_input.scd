MIDIClient.init;
MIDIIn.connectAll;

"Available MIDI sources:".postln;
MIDIClient.sources.do({ |src, i|
    [i, src].postln;
});

MIDIdef.noteOn(\projectNoteOn, {
    |velocity, note, channel, source|
    if (velocity == 0) {
        // Specchia a Processing: stato 0 = rilasciato
        ~processing.sendMsg("/vmpkNota", 0, note, velocity);

        ~handleNoteOff.value(note, velocity, channel, source);
    } {
        // Specchia a Processing: stato 1 = premuto
        ~processing.sendMsg("/vmpkNota", 1, note, velocity);

        ~handleNoteOn.value(note, velocity, channel, source);
    };
});


// ============================================================
// NOTE OFF
// ============================================================

MIDIdef.noteOff(\projectNoteOff, {
    |velocity, note, channel, source|
    ~handleNoteOff.value(note, velocity, channel, source);
});

MIDIdef.cc(\projectCC, {
    |value, ccNumber, channel, source|
    ~handleCC.value(value, ccNumber, channel, source);
});



// ============================================================
// PITCH BEND — joystick orizzontale MPK
// Permette la microtonalita' controllata fisicamente
// ============================================================

MIDIdef.bend(\projectBend, {
    |val, channel, source|
    ~handlePitchBend.value(val, channel, source);
});


"MIDI input loaded with OSC mirroring.".postln;
