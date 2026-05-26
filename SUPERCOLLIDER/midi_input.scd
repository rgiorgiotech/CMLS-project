/*
    MIDI INPUT
    Riceve tutti i messaggi MIDI dalla MPK e li smista al mapping layer.
    Specchia gli eventi a Processing via OSC per la grafica in tempo reale.
*/

MIDIClient.init;
MIDIIn.connectAll;

"Available MIDI sources:".postln;
MIDIClient.sources.do({ |src, i|
    [i, src].postln;
});

// Sicurezza: se ~processing non è ancora definito in config.scd,
// lo inizializziamo qui sulla porta 12000
~processing = ~processing ?? { NetAddr("127.0.0.1", 12000) };


// ============================================================
// NOTE ON
// ============================================================

MIDIdef.noteOn(\projectNoteOn, {
    |velocity, note, channel, source|

    // MIDI convention: NoteOn con velocity 0 equivale a NoteOff
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

    // Specchia a Processing: stato 0 = rilasciato
    ~processing.sendMsg("/vmpkNota", 0, note, velocity);

    ~handleNoteOff.value(note, velocity, channel, source);
});


// ============================================================
// CC (Control Change) — knob MPK
// ============================================================

MIDIdef.cc(\projectCC, {
    |value, ccNumber, channel, source|

    // Specchia i CC a Processing per visualizzazione
    ~processing.sendMsg("/vmpkCC", ccNumber, value);

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
