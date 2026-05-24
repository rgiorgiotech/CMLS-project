/*
    MAIN ENTRY POINT
    Run this file.
*/

(
~projectPath = thisProcess.nowExecutingPath.dirname;

(~projectPath +/+ "cleanup.scd").load;
(~projectPath +/+ "config.scd").load;

s.waitForBoot({

    (~projectPath +/+ "synths.scd").load;
    s.sync;

    (~projectPath +/+ "mapping.scd").load;
    (~projectPath +/+ "midi_input.scd").load;

    "============================================================".postln;
    "PROJECT BACKBONE READY.".postln;
    "Play your virtual piano or MIDI keyboard.".postln;
    "============================================================".postln;
});
)