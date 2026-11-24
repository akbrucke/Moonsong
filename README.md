![Static Badge](https://img.shields.io/badge/Version-1.10-blue)

# Moonsong
VST3 Vocal Delay &amp; Reverb Console using JUCE Framework, vibe-coded and then refined with a human touch

## Overview & Design Philosophy
Moonsong is a dedicated vocal spatial workstation engineered to deliver lush, three-dimensional depth
without clouding the lead vocal or cluttering the low-midrange of a dense mix. Rather than chaining
separate dynamics, stereo wideners, delays, and reverbs across multiple insert slots, Moonsong
integrates a complete vocal ambience chain inside a single focused interface.

Incoming audio is conditioned up front by an optical-style compressor and an optional stereo doubling
stage (Eclipse Mode) before splitting into parallel Delay and Reverb engines. Independent high-pass,
low-pass, broad-bell/high-shelf EQ, and saturation modules can be toggled either Pre-FX or Post-FX for
each engine, while a post-FX stereo chorus adds further dimension.

The plugin features a Mix control, so that it can be used either as a direct insert, or as a send effect.

## Features
Pre-FX Optical Compressor: Combines fast (60 ms) and slow (650 ms) release stages to keep
breathy vocal phrases consistent as they enter the delay and reverb engines.

Eclipse Mode Stereo Doubler: Positioned post-compression and pre-FX so the spatial engines react
to a wide stereo image rather than a narrow mono center.

Mud-Free 8-Channel FDN Reverb: Uses 4 series all-pass diffusers feeding an 8-tank Householder
feedback matrix with a built-in 135 Hz structural high-pass filter inside the tank loop to prevent low-mid
vocal buildup.

### Delay Modes & 5-Tap Panner Matrix
The central Delay module operates in standard stereo modes or morphs directly into a dedicated 5-tap
spatial panner when MULTI-TAP is engaged.
Modes included: Ping Pong, Dual Mono, Mono, and Multi-Tap.

## Screenshots
Three collapsible drawers frame the central console, keeping the main workspace clean until additional
vocal shaping is needed. Click any drawer tab to smoothly slide it open or closed. The window can be freely
resized.  

<img width="922" height="580" alt="Screenshot 2026-11-23 035326" src="https://github.com/user-attachments/assets/6debce8d-d2ba-4655-b8d7-6dffcc3ecca5" />
<img width="1248" height="845" alt="Moonsong" src="https://github.com/user-attachments/assets/356df091-a490-486d-9d17-6a1543162248" />

## Installation
Currently, Moonsong is released for Windows 10/11 only, but Mac and Linux releases are coming soon. To grab the latest prebuilt release, head to the [releases page.](https://github.com/akbrucke/Moonsong/releases) Included with the release download is a thorough user manual, a collection of presets, and installation instructions.

## Building from Source (Windows)

Moonsong uses CMake and automatically fetches JUCE (`9.0.5`) during configuration. No manual SDK downloads or git submodules are required.

### Prerequisites
* **Git**
* **CMake 3.22 or newer**
* **Visual Studio 2022** (Community Edition or newer) with the **Desktop development with C++** workload installed

### Build Steps

1. Clone the repository
   
2. Open a terminal in the root folder and run  
`cmake -B build -G "Visual Studio 17 2022" -A x64`  
And then compile the release build:  
`cmake --build build --config Release`

3. Locate & Install the VST3 Plugin  
The finished VST3 bundle will be located in `build\Moonsong_artefacts\Release\VST3\Moonsong.vst3`  
Copy `Moonsong.vst3` into your system VST3 folder so your DAW can detect it.  
On Windows, this would be: `C:\Program Files\Common Files\VST3\`

## Building from Source (macOS, Apple silicon)

### Prerequisites
* **Xcode Command Line Tools**  
* **Homebrew**  
* **CMake 3.22 or newer**  
* **Ninja**  
(Full Xcode is not required)  

### Build Steps

1. Clone the repository  

2. Open a terminal in the root folder and run  
`cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`   
then `cmake --build build`  

3. Locate & Install the VST3 Plugin  
The finished VST3 bundle will be located in `build\Moonsong_artefacts\Release\VST3\Moonsong.vst3`  
Copy `Moonsong.vst3` into your system VST3 folder so your DAW can detect it.  
On macOS, this would be: `~/Library/Audio/Plug-Ins/VST3/`  

- *Note that Logic Pro and GarageBand only load Audio Units, so they won't see the VST3*

## Changelog
MOONSONG V1.10 (November 22 2026)  
-Added a resizable window so that Multi-Tap controls can still be visible with all three drawers open  
-Changed spacing of logo and subtitle from the left edge for a cleaner look  
-Updated font rendering in preparation for cross-platform support  

MOONSONG V1.00 (November 21 2026)  
-Released  
