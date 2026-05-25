/*
    CONFIGURATION
*/

~juceIP = "127.0.0.1";
~jucePort = 9001;
~juceAddress = NetAddr(~juceIP, ~jucePort);

~processingIP = "127.0.0.1";
~processingPort = 12000;
~processingAddress = NetAddr(~processingIP, ~processingPort);


// ============================================================
// NOTE / PERFORMANCE CONFIGURATION
// ============================================================

~maxVoices = 6;

// MIDI register zones
~lowRegisterMax = 48;   // C3 and below
~midRegisterMax = 72;   // C3-C5 approximately
~highestAllowedNote = 96;

// Velocity to amplitude mapping
~noteVelocityAmpMin = 0.15;
~noteVelocityAmpMax = 0.80;

// Safety release if noteOff is lost
~noteSafetyReleaseTime = 3.0;

// ============================================================
// TOUCHPAD CONFIGURATION
// ============================================================

// Touchpads send MIDI CC, not MIDI notes.
~touchpadCCs = [20, 21, 22, 23, 24, 25, 26, 27];

// Threshold: values above this count as "pressed".
~touchpadThreshold = 0.05;

// Stores whether each pad is currently physically pressed.
// Used to avoid repeated toggles while touching/holding.
~touchpadPressed = Array.fill(8, { false });

~layerNames = [
    "SINE",
    "SAW",
    "PULSE",
    "FM",
    "NOISE",
    "SUB",
    "GLITCH",
    "PAD"
];

~layerGains = [
    0.35,
    0.25,
    0.22,
    0.25,
    0.18,
    0.30,
    0.18,
    0.22
];