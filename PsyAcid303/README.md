# Psy Acid 303 (VST3)

Acid-Psytrance-Synthesizer im TB-303-Stil fuer Goa- und Psytrance.

## Funktionen
- Monophoner 303-Synth: Saw/Square, resonanter Ladder-Filter, Cutoff, Resonance, Env Mod, Decay, Accent, Slide, Tune, Volume
- 16-Step-Sequenzer mit Note, Gate, Accent und Slide pro Step
- **Step Distortion**, **Step Filter**, **Step LFO** (pro Step einstellbar)
- LFO (Sine, Triangle, Saw, Square, S&H) -> Cutoff / Resonance / Distortion / Pitch
- **Alles BPM-getaktet**: Sequenzer, LFO und Pan Delay (1/1 bis 1/16T, punktiert, Triolen)
- Ping-Pong **Pan Delay** mit Feedback, Mix, Pan-Width und Tone
- Synchron zur DAW: folgt Transport, Tempo und Songposition. Ohne laufende DAW: "Free Run" mit "Int BPM"
- **100 interne Presets** (10 Kategorien x 10 Variationen), jedes mit eigenem Pattern
- Psytrance-Oberflaeche, skalierbar

## MIDI
- Sequenzer aktiv: gehaltene Taste transponiert das Pattern relativ zu C3 (MIDI 60 = keine Transposition)
- Sequenzer aus / DAW gestoppt ohne Free Run: Tasten spielen den Synth direkt

## Bauen mit GitHub
1. Den Inhalt dieses Ordners in ein neues GitHub-Repository hochladen (Ordner `.github` muss dabei sein).
2. Tab **Actions** -> Workflow **Build VST3** startet automatisch (oder "Run workflow").
3. Nach dem Build unter **Artifacts** die VST3 fuer Windows, macOS oder Linux herunterladen.
4. Optional: Tag `v1.0.0` pushen -> der Workflow erstellt automatisch ein GitHub Release mit ZIPs.

## Installation
- Windows: `Psy Acid 303.vst3` nach `C:\Program Files\Common Files\VST3`
- macOS: nach `~/Library/Audio/Plug-Ins/VST3` (unsigniert: ggf. `xattr -cr "Psy Acid 303.vst3"`)
- Linux: nach `~/.vst3`

## Lokal bauen
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Lizenz-Hinweis
Nutzt das JUCE-Framework (AGPLv3 oder kommerzielle JUCE-Lizenz).
