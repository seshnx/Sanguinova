# SeshNx Sanguinova

**Blood Star Distortion** - A characterful asymmetric distortion plugin with combinatorial drive stages.

**Part of the SeshNx Plugin Suite by Amalia Media LLC**

## Features

- **Asymmetric Waveshaping**: Tube-like saturation on positive peaks, gritty compression on negative peaks
- **Ignition Stages**: Three combinatorial multipliers (2x, 5x, 10x) for up to 100x overdrive
- **Color Filter**: Multi-mode SVF pre-filter (Low Pass, High Pass, Band Pass) with Q control
- **Pad Compensation**: Automatic gain compensation based on multiplier level with soft release
- **4x Oversampling**: Anti-aliasing for clean, artifact-free distortion
- **Real-time Oscilloscope**: Visual waveform display
- **Post-Filter**: 1-pole low-pass for smoothing harsh harmonics

## Signal Flow

```
Input → Pre-Filter (SVF) → 4x Oversampling → Distortion Engine
      → Post-Filter (LPF) → Pad → Output Gain → Wet/Dry Mix → Output
```

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| INPUT Q | 0.1 - 1.0 | Pre-filter resonance |
| COLOR | 20 Hz - 20 kHz | Pre-filter frequency |
| FILTER MODE | LP/HP/BP | Pre-filter type |
| DRIVE | 0 - 40 dB | Pre-amp gain |
| STAGE I | 2x | First multiplier stage |
| STAGE II | 5x | Second multiplier stage |
| STAGE III | 10x | Third multiplier stage |
| POST-FILTER | 2 kHz - 20 kHz | Output low-pass cutoff |
| TRIM | -12 to +12 dB | Output gain |
| PAD | On/Off | Automatic gain compensation |
| MIX | 0 - 100% | Wet/dry blend |

## Build Formats

- **VST3** (Windows, macOS, Linux)
- **AU** (macOS)
- **Standalone** (All platforms)
- **AAX** (With AAX SDK)

## Building

### Requirements
- CMake 3.15+
- C++17 compiler
- JUCE 7.0.9 (fetched automatically)

### Build Commands

```bash
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

### Output Locations
- VST3: `build/Sanguinova_artefacts/Release/VST3/`
- Standalone: `build/Sanguinova_artefacts/Release/Standalone/`
- AU (macOS): `build/Sanguinova_artefacts/Release/AU/`

## Version

**v1.0.0**

---

## License

Copyright (c) 2024 Amalia Media LLC. All rights reserved.

Proprietary software - Distribution prohibited without explicit permission.

---

## Support

For technical support, bug reports, or feature requests, please contact the development team through official SeshNx channels.

---

*Part of the [SeshNx Plugin Suite](https://seshnx.com) by Amalia Media LLC*
