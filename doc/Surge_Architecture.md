# Surge Architecture

This guide outlines how Surge fits together; it is not a developer how-to. See the
[Developer Guide](./Developer%20Guide.md) for development practices. This overview is
informal, incomplete, and still being written.

- [Code Layout](#code-layout)
- [SurgeStorage, Parameter and SurgeSynthesizer](#surgestorage-parameter-and-surgesynthesizer)
- [SurgeGUIEditor](#surgeguieditor)
- [Voices and MPE](#voices-and-mpe)
- [DSP](#dsp)
- [Headless](#headless)

## Code Layout

Third-party libraries live in `libs`, either copied into the repository or included as
submodules. Surge's C++ source is in `src`:

- `src/common` contains the engine, DSP, and voice-handling code.
- `src/surge-xt/gui` contains the UI, custom widgets, and helpers.
- `src/surge-xt/plugin_type_extensions` contains JUCE wrappers for each plugin format.
- `src/surge-testrunner` contains the headless build.

## SurgeStorage, Parameter and SurgeSynthesizer

Surge components coordinate through `SurgeStorage`, a named collection of `Parameter`
objects. Each parameter stores a type and value and defines how changes update the
system. At runtime, `SurgeSynthesizer` reads values from the current `SurgeStorage`.

Loading a patch streams its data into the synthesizer's current storage. A UI change
locates the relevant parameters and calls the appropriate value-change function to
invalidate them.

## SurgeGUIEditor

`SurgeGUIEditor` works with the parameter set. Its `idle` thread reflects external
changes, such as changes delivered by the host, in the UI.

The available oscillator types are Classic and FM3 (shown as Sine FM). Their original
type IDs remain unchanged. Other oscillator types in loaded patches are replaced with
Classic with a warning; automation targeting an unavailable type selects Classic.
Wavetable file drops are disabled because no wavetable oscillator is available.
Legacy oscillator IDs and some unused GUI definitions remain for patch and interface
compatibility; the other oscillator DSP implementations are not built.

## Voices and MPE

Voice allocation and stealing support multiple polyphony modes and MPE. Voices are
allocated at startup and configured with an in-place constructor; each holds a reference
to the state for its playing channel.

The voice class is `SurgeVoice` in `src/common/dsp/SurgeVoice.h`, and contains a
`SurgeVoiceState`.

## DSP

Most DSP operations use hand-coded SSE2 for speed. As a result, data structures must be
aligned on 16-byte boundaries.

The scene waveshaper offers Soft, Asymmetric, and OJD. Double-click its waveform or use
the Enabled menu item to bypass it. Legacy Off patches load with Soft selected and
bypassed; unavailable types are replaced with Soft and reported. Automation targeting an
unavailable type selects Soft without changing bypass. The shared waveshaper library is
also used by other DSP, including the LFO and Ensemble.

## Headless

The headless host runs Surge without a UI and is used primarily for debugging and tests.
