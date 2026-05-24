/*
    CONFIGURATION
*/

~juceIP = "127.0.0.1";
~jucePort = 9001;

~juceAddress = NetAddr(~juceIP, ~jucePort);

~processingIP = "127.0.0.1";
~processingPort = 12000;

~processingAddress = NetAddr(~processingIP, ~processingPort);

~activeNotes = IdentityDictionary.new;

"Configuration loaded.".postln;