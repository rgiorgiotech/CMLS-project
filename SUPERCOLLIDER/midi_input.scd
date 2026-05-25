/*
    RAW MIDI INPUT LAYER

    Purpose:
    - Initialize MIDI.
    - Receive raw MIDI events.
    - Forward them to mapping.scd handlers.

    Important:
    This file should not contain musical decisions.
*/

MIDIClient.init;
MIDIIn.connectAll;

"Available MIDI sources:".postln;
MIDIClient.sources.do({ |src, i|
    [i, src].postln;
});


// ============================================================
// NOTE ON
// ============================================================

MIDIdef.noteOn(\projectNoteOn, {
    |velocity, note, channel, source|

    // MIDI convention:
    // NoteOn with velocity 0 is equivalent to NoteOff.
    if (velocity == 0) {
        ~handleNoteOff.value(note, velocity, channel, source);
    } {
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


// ============================================================
// CC
// Left unchanged for now.
// ============================================================

MIDIdef.cc(\projectCC, {
    |value, ccNumber, channel, source|

    ~handleCC.value(value, ccNumber, channel, source);
});


"MIDI input loaded.".postln;