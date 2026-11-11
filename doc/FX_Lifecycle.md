# Effects lifecycle

This note describes effect creation, replacement, and initialization as of commit
`8e03ba0b54b09` or thereabouts.

Only Ensemble, Delay, and Reverb 1 can be selected or instantiated. Other effect IDs
remain reserved so the retained effects keep their original patch and automation IDs.
Loading a patch or FX preset using an unavailable effect turns that slot off and reports
a warning; automation targeting an unavailable ID turns the slot off. Presets for
unavailable effects are omitted from the menu. Most unused effect implementations have
been removed; Conditioner and Convolution still build for their legacy GUI code, but
cannot be instantiated.

## Core structures and functions

- `spawn_effect` creates an `Effect` of the requested type, bound to storage and data.
- `loadFx` compares the synth and runtime states and optionally reloads effects:
  - `initp=true` calls `init_default_values`; otherwise, parameters are forced into
    their default ranges.
  - `force_reload_all=true` reloads every effect.
  - On a type change, `loadFx` first sets all parameters to type `none`.
  - It is called by `synth::processAudioThreadOpsWhenAudioEngineUnavailable` and
    `synth::processControl` with `false, false`, and by `synth::loadRaw` with
    `false, true` to force a full reload after patch loading.
  - If either kind of reload changes an effect, its virtual `updateAfterReload()` runs.
- `SurgeSynth::load_fx_needed` triggers `loadFx` in `processControlEV`. It is set in the
  synth constructor, set on `ct_fxtype` changes in `setParameter01` (the automation
  path), and set by `SurgeGUIEditor` for `tag_fx_menu`; `loadFx` clears it.
- The patch's `fx[n_fx]` array holds `FXStorage` pointers.
- The synth's `fxsync` is allocated as `n_fx_slots * sizeof(FXStorage)` and serves as an
  edit buffer for external editors that update effect types in bulk:
  - The synth constructor points each slot to `patch.fx[i]`.
  - `SurgeGUIEditor` edits `fxsync` through `XMLConfiguredMenus`, not the patch buffer.
  - When reloading an effect, `loadFx` copies sync values to the patch, treating them as
    the values set by the GUI. If `fx_reload[s]` is true, it also copies from sync,
    suspends, and initializes the effect.
- `SurgeGUIEditor` uses `fx_reload[n_fx_slots]` to force a reload when the effect type
  has not changed.

## Empty effect to loaded effect, effect enabled

`spawn_effect` is called twice: once from the snapshot menu, and once from
`SurgeSynthesizer::loadFx`. The first call's stack trace:

```
Spawn Effect  id=6
-------- StackTrace (7 frames of 40 depth showing) --------
  [  1]: 1   Surge                               0x000000015000dc72 _Z12spawn_effectiP12SurgeStorageP9FxStorageP5pdata + 130
  [  2]: 2   Surge                               0x0000000150214423 _ZN7CFxMenu12loadSnapshotEiP12TiXmlElementi + 227
  [  3]: 3   Surge                               0x000000015022bc4e _ZZN13CSnapshotMenu30populateSubmenuFromTypeElementEP12TiXmlElementPN6VSTGUI11COptionMenuERiS5_RKlS5_ENK3$_0clEPNS2_16CCommandMenuItemE + 62
  [  4]: 4   Surge                               0x000000015022bbf2 _ZNSt3__1L8__invokeIRZN13CSnapshotMenu30populateSubmenuFromTypeElementEP12TiXmlElementPN6VSTGUI11COptionMenuERiS7_RKlS7_E3$_0JPNS4_16CCommandMenuItemEEEEDTclclsr3std3__1E7forwardIT_Efp_Espclsr3std3__1E7forwardIT0_Efp0_EEEOSE_DpOSF_ + 50
  [  5]: 5   Surge                               0x000000015022bb92 _ZNSt3__128__invoke_void_return_wrapperIvE6__callIJRZN13CSnapshotMenu30populateSubmenuFromTypeElementEP12TiXmlElementPN6VSTGUI11COptionMenuERiS9_RKlS9_E3$_0PNS6_16CCommandMenuItemEEEEvDpOT_ + 50
  [  6]: 6   Surge                               0x000000015022bb42 _ZNSt3__110__function12__alloc_funcIZN13CSnapshotMenu30populateSubmenuFromTypeElementEP12TiXmlElementPN6VSTGUI11COptionMenuERiS8_RKlS8_E3$_0NS_9allocatorISB_EEFvPNS5_16CCommandMenuItemEEEclEOSF_ + 50
Spawn Effect  id=6
-------- StackTrace (7 frames of 38 depth showing) --------
  [  1]: 1   Surge                               0x000000015000dc72 _Z12spawn_effectiP12SurgeStorageP9FxStorageP5pdata + 130
  [  2]: 2   Surge                               0x0000000150107b73 _ZN16SurgeSynthesizer6loadFxEbb + 1155
  [  3]: 3   Surge                               0x000000015010c5f3 _ZN16SurgeSynthesizer14processControlEv + 3667
  [  4]: 4   Surge                               0x000000015010ccb2 _ZN16SurgeSynthesizer7processEv + 1234
  [  5]: 5   Surge                               0x000000015063eb2d _ZN7aulayer6RenderERjRK14AudioTimeStampj + 1277
  [  6]: 6   Surge                               0x0000000150640620 _ZN6AUBase9RenderBusERjRK14AudioTimeStampjj + 96
```

`XMLConfiguredMenus::loadSnapshot` creates an effect, calls `init_ctrltypes()` and
`init_default_values()`, then deletes the effect. It parses the XML into the `fxbuffer`,
leaving configured values and types but no effect instance. It then raises `valueChanged`,
causing `SurgeGUIEditor` to set `load_fx_needed`. On the next `process()` call,
`SurgeSynthesizer::processControl` calls `loadFx(false, false)` and spawns the effect.
Because `initp` is false, it retains the values left by `XMLConfiguredMenus`.

If audio processing is inactive, the UI also initializes the effect through
`synth->processAudioThreadOpsWhenAudioEngineUnavailable()`; this note does not cover that
case further.

## Other transitions

- **Loaded effect to disabled, effect enabled:** `XMLConfiguredMenus` calls
  `loadSnapshot` with type 0 and null XML, sets `valtype` to 0, and notifies
  `SurgeGUIEditor`. What sets all types to `none`?
- **Empty effect to loaded effect, effect bypassed**
- **Active effect to new preset of same type, effect on:** `XMLConfiguredMenus`
- **Active effect to new effect of different type, effect on**
- **Active effect to new effect, effect bypassed:** Is bypass from CEG different from
  global bypass?
- **Load a patch with effect off**
- **Load a patch with active effect**
- **Load a user effect setting:** `FXMenu::loadUserPreset` resembles `loadSnapshot`: it
  creates an effect on the sync buffer, applies preset values, and raises an event. The
  rest follows the same-type load path.
- **Swap an effect**

## DAW automation

The DAW automation path enters through `synth::setParameter01`; the `ct_fxtype` case
spawns and replaces the effect. See its comments for details.
