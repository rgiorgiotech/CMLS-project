/*
    CLEANUP
*/

MIDIdef.freeAll;

// Release old Synth objects
if (~activeSynths.notNil) {
    ~activeSynths.do({ |synth|
        synth.set(\gate, 0);
    });

    ~activeSynths.clear;
};

// Clear musical note state
if (~activeNotes.notNil) {
    ~activeNotes.clear;
};

"Cleanup done.".postln;