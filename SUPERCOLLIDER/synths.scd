/*
    SYNTH DEFINITIONS
*/

SynthDef(\mainSynth, {
    |out = 0, freq = 440, amp = 0.2, gate = 1|
    var sig, env;

    env = EnvGen.kr(
        Env.adsr(
            attackTime: 0.01,
            decayTime: 0.15,
            sustainLevel: 0.6,
            releaseTime: 0.6
        ),
        gate,
        doneAction: 2
    );

    sig = Saw.ar(freq);
    sig = LPF.ar(sig, (freq * 3).min(8000));
    sig = sig.tanh;
    sig = sig * env * amp;
    sig = sig.clip(-0.8, 0.8);

    Out.ar(out, sig ! 2);
}).add;


~maxVoices = 6;

~playSynth = { |note, freq, amp|
    var synth;

    // Rilascia synth precedente sulla stessa nota
    if (~activeNotes[note].notNil) {
        ~activeNotes[note].set(\gate, 0);
        ~activeNotes.removeAt(note);
    };

    // Limita il numero massimo di voci attive
    if (~activeNotes.size >= ~maxVoices) {
        var oldestKey = ~activeNotes.keys.asArray.first;
        ~activeNotes[oldestKey].set(\gate, 0);
        ~activeNotes.removeAt(oldestKey);
    };

    synth = Synth(\mainSynth, [
        \freq, freq,
        \amp, amp,
        \gate, 1
    ]);

    ~activeNotes[note] = synth;

    // Safety timer — forza il release dopo 3 secondi
    // nel caso il noteOff non arrivi mai
    SystemClock.sched(3.0, {
        if (~activeNotes[note].notNil) {
            ~activeNotes[note].set(\gate, 0);
            ~activeNotes.removeAt(note);
            ("Safety release nota: " ++ note).postln;
        };
        nil;
    });
};

~releaseSynth = { |note|
    if (~activeNotes[note].notNil) {
        ~activeNotes[note].set(\gate, 0);
        ~activeNotes.removeAt(note);
    };
};

"Synths loaded.".postln;
