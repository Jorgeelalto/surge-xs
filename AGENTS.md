# Repository guidance

- Do not git add, commit or push without asking first.

## Primary references

For every code change, read and follow the [Developer Guide](doc/Developer_Guide.md) for coding style, naming, comments, formatting, and general implementation conventions. Also consult [Surge Architecture](doc/Surge_Architecture.md) for the code layout and the relationships between storage, parameters, the synthesizer, GUI, voices, DSP, and the headless host. The architecture document is an evolving overview, not a complete implementation guide.

## Build validation

On macOS, validate builds using only the VST3 and standalone targets by default: `cmake --build <build-dir> --target surge-xt_VST3 surge-xt_Standalone`. Avoid building all targets unless the task specifically requires broader coverage.

## Documentation index

The Markdown documents in [`doc/`](doc/) are repository references. Read the relevant documents before changing the features or workflows they describe; these pages contain the detailed procedures and design notes.

- [Developer Guide](doc/Developer_Guide.md) — coding conventions, C++ naming and comments, formatting, platform-specific implementation guidance, and miscellaneous practices.
- [Surge Architecture](doc/Surge_Architecture.md) — source layout and high-level architecture, including `SurgeStorage`, `Parameter`, `SurgeSynthesizer`, the GUI editor, voice allocation and MPE, DSP/SSE alignment, and headless operation.
- [Adding a Filter](doc/Adding_a_Filter.md) — filter implementation and registration across DSP files, filter configuration, coefficient generation, SIMD processing, subtype stringification, and voice setup.
- [Adding an Effect](doc/Adding_an_FX.md) — effect implementation, parameter setup, spawning and type registration, configuration XML, presets, and build integration.
- [Effects Lifecycle](doc/FX_Lifecycle.md) — effect creation, reload and replacement behavior, storage and edit buffers, snapshots, bypass scenarios, and DAW automation.
- [Adding an SVG](doc/Adding_an_SVG.md) — adding SVG resources to the resource bundle and wiring platform-specific resource identifiers and skin image maps.
- [Dynamic Names, Deactivation and Bipolarity](doc/Dynamic_Names_Deactivation_and_Bipolarity.md) — dynamic parameter labels, activation clusters, display behavior, and bipolar formatting constraints.
- [Surge Patch Tag Data Model](doc/Patch_Tag_Data_Model.md) — the proposed/RFC physical and logical patch-tag model, including tag categories, cardinality, and examples.
- [Surge Wavetables](doc/Wavetables.md) — `.wt` wavetable structure and the `wt-tool` workflows for inspecting, extracting, and creating wavetable files.