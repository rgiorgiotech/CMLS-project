MIDIClient.init;
MIDIIn.connectAll;

"Available MIDI sources:".postln;
MIDIClient.sources.do({ |src, i|
    [i, src].postln;
});

MIDIdef.noteOn(\projectNoteOn, {
    |velocity, note, channel, source|
    if (velocity == 0) {
        ~handleNoteOff.value(note, velocity, channel, source);
    } {
        ~handleNoteOn.value(note, velocity, channel, source);
    };
});

MIDIdef.noteOff(\projectNoteOff, {
    |velocity, note, channel, source|
    ~handleNoteOff.value(note, velocity, channel, source);
});

MIDIdef.cc(\projectCC, {
    |value, ccNumber, channel, source|
    ~handleCC.value(value, ccNumber, channel, source);
});

"MIDI input loaded.".postln;