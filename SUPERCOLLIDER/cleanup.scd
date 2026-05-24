/*
    CLEANUP
*/

MIDIdef.freeAll;

if(~activeNotes.notNil) {
    ~activeNotes.do({ |synth|
        synth.set(\gate, 0);
    });

    ~activeNotes.clear;
};

"Cleanup done.".postln;