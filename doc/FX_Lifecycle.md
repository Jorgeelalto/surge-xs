# Effects lifecycle

Surge XS has one fixed, shared chain after the scene outputs are mixed:
Ensemble → Delay → Reverb → Conditioner. Each effect is always present, and its Mix
parameter starts at 0% wet. The four selector buttons choose which effect's
parameters appear in the editor; there are no FX type, preset, send, or bypass
controls.

The chain uses the existing four global FX slots, preserving the original
slot and parameter IDs. Other FX slots and the old send and bypass parameters
remain in storage for compatibility but do not participate in audio processing.
The effects use the existing ringout handling in serial order.

New patch and host-state saves carry the `xs_fixed_fx="2"` marker and preserve FX
parameters and their modulation. Older patches, including three-effect saves with
`xs_fixed_fx="1"`, have their FX configuration and FX modulation discarded and the
fixed chain initialized with its defaults. Other patch settings are retained.

Conditioner has 64 samples of lookahead. Both its wet and dry paths have that
delay, including at 0% Mix. The plugin reports 64 samples of latency to the host;
the optional pre-FX scene output buses are delayed by the same amount.
Its red meter shows limiter gain reduction in the processed path, including
when Mix is fully dry.

`SurgeSynthesizer::loadFx` still synchronizes effect storage and runtime
instances. Effect type automation cannot replace the fixed types, and the
editor's current slot is restricted to the four fixed slots.
