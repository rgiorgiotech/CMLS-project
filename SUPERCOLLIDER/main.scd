/*
    MAIN ENTRY POINT
    Run this file.
*/

/*
    MAIN ENTRY POINT
    Run this file.
*/

(
~projectPath = thisProcess.nowExecutingPath.dirname;

(~projectPath +/+ "cleanup.scd").load;
(~projectPath +/+ "config.scd").load;
(~projectPath +/+ "note_state.scd").load;

s.waitForBoot({

    (~projectPath +/+ "synths.scd").load;
    s.sync;

    (~projectPath +/+ "mapping.scd").load;
    (~projectPath +/+ "midi_input.scd").load;

    "============================================================".postln;
    "PROJECT BACKBONE READY.".postln;
    "MIDI note layer improved: velocity, duration, register, polyphony, legato, repeated notes.".postln;
    "Play your virtual piano or MIDI keyboard.".postln;
    "============================================================".postln;
});
)