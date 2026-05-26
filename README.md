# 🍰 LadyfingerS Morph Delay - Tiramisù Edition

![CMLS](https://img.shields.io/badge/Course-Computer_Music_Languages_and_Systems-blue)
![Polimi](https://img.shields.io/badge/University-Politecnico_di_Milano-orange)
![C++](https://img.shields.io/badge/C++-JUCE_Framework-00599C?logo=c%2B%2B)
![SuperCollider](https://img.shields.io/badge/SuperCollider-Audio_Engine-DD3322)

**LadyfingerS Morph Delay** is an interactive computer music instrument that seamlessly integrates granular synthesis (SuperCollider), a custom C++ DSP effects chain (JUCE), and a reactive visualizer (Processing). It is designed to be played live using a MIDI keyboard (AKAI MPK Mini Mk II).

The UI and conceptual design are heavily inspired by the "Ladyfingers TiraSynth-8", featuring a custom bakery-themed GUI drawn entirely from scratch in C++.

---

## 🚀 Features

* **Macro-Morphing Paradigm:** A single gesture (the "Espresso" slider) simultaneously transforms 5 delay parameters (Time, Feedback, Damping, Wet, Dry), morphing the sound from ambient textures to extreme glitch.
* **Dynamic Microtonality:** The physical joystick is mapped to a ±100 cents granular pitch shifter, enabling continuous microtonal expression on a standard MIDI keyboard.
* **Envelope-Reactive DSP:** An internal envelope follower tracks your playing amplitude to auto-modulate the delay feedback in real-time.
* **Perfect Sync:** High-level OSC routing synchronizes the physical inputs, the DSP engine, and the visual feedback with zero audible latency.

## ⚙️ System Architecture

The project is built on a hybrid, tightly coupled pipeline:

1.  **Input:** AKAI MPK Mini Mk II sends MIDI data.
2.  **SuperCollider (Port 9001/12000):** Acts as the core synthesis engine (TGrains) and the OSC routing hub. It parses MIDI, calculates polyphony and musical states, and broadcasts control data.
3.  **Virtual Audio Cable:** Routes SC audio output directly into the JUCE plugin.
4.  **JUCE Plugin:** Processes the audio through a Saturator, a Morphing Stereo Delay, and a Dual Read-Head Pitch Shifter.
5.  **Processing:** Receives OSC data to generate a real-time reactive particle system.

## 🛠️ Prerequisites

To run this project, you will need:
* **SuperCollider** 3.14 or higher
* **JUCE** 8.0.12 (if compiling from source)
* **Processing** 4+ (with `oscP5` library installed)
* **VB-Audio Virtual Cable** (Free)
* A MIDI Keyboard (configured for AKAI MPK Mini Mk II mappings)

## 🏁 Getting Started

Due to Windows MIDI exclusivity constraints, **startup order is strictly required**.

1.  Connect your MIDI Keyboard via USB.
2.  Ensure **VB-Audio Virtual Cable** is running.
3.  Open `SuperCollider` and run the master entry point:
    ```supercollider
    // In SC, open main.scd and evaluate the block
    (~projectPath +/+ "main.scd").load;
    ```
    *Wait for the `PROJECT BACKBONE READY` message in the post window.*
4.  Open the compiled **LadyfingerS JUCE Standalone application** (or load the VST3 in your DAW).
5.  In the Plugin Audio Settings, route the audio:
    * *Input:* `CABLE Output (VB-Audio Virtual Cable)`
    * *Output:* Your Main Speakers/Headphones
6.  Open Processing, run `gui_processing.pde`, and enjoy the visuals!

## 🎛️ Interaction Mapping

| Physical Control | Function | Target |
| :--- | :--- | :--- |
| **Piano Keys** | Note On/Off (Granular Synth) | SuperCollider |
| **Pads 1-8** | Toggle Synth Layers (Sine, Saw, Glitch...) | SuperCollider |
| **Knobs 1-7** | Morph, Wet, Dry, Feedback, Time, Damping, Drive | JUCE DSP via OSC |
| **Knob 8** | Coarse Pitch (±24 Semitones) | JUCE Pitch Shifter |
| **Joystick (X-Axis)** | Fine Pitch (±100 Cents) | JUCE Pitch Shifter |

## 🧠 Core Code Snippets

### JUCE: Pitch Shifter Crossfading
To avoid artifacts during pitch manipulation, the Pitch Shifter uses two read-heads with sinusoidal crossfading:
```cpp
float dist1 = std::fmod((writeHead - readHead1 + bufSize), (float)bufSize) / bufSize;
float fade1 = std::sin(juce::MathConstants<float>::pi * dist1);
```

### SuperCollider: OSC Routing
SC acts as the brain, parsing raw MIDI and sending semantic OSC data to JUCE:
```supercollider
~sendNoteOnToJuce = { |info|
    ~juceAddress.sendMsg(
        "/note/on", info[\note], info[\freq], info[\amp], 
        info[\velocityNorm], info[\polyphony]
    );
};
```

## 📝 License & Credits
Developed for the **Computer Music - Languages and Systems** course at Politecnico di Milano (A.Y. 2025/2026).
