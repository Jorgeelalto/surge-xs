# Adding an effect

This guide uses `MyEffect` as an example.

1. Create `src/common/dsp/effects/MyEffect.h` and `MyEffect.cpp`. See Delay for an
   example. In the header, define a named enum for the parameters. Override
   `init_ctrltypes()` to set their names and types, `init_default_values()` to set their
   defaults, and implement signal processing in `process()`.

2. In `src/common/dsp/Effect.cpp`, include the new header and add a case to
   `spawn_effect()`:

   ```cpp
   case fxt_myeffect:
       return new MyEffect(storage, fxdata, pd);
   ```

   This example uses the effect ID `fxt_myeffect`.

3. In `SurgeStorage.h`, add the ID to `fx_type`, allow it in `isAvailableFxType`, and add the effect's name to
   `fx_type_names`, its short name to `fx_type_shortnames`, and its 3- or 4-character
   acronym to `fx_type_acronyms`. The short name must fit in the effect-type menu; check
   that the acronym fits in the GUI FX grid boxes.

4. Add the effect to the `fx` XML group in `resources/surge-shared/configuration.xml`.
   Add snapshots for its presets if needed: put `"init"` there and additional factory
   presets in `resources/data/fx_presets`.

5. In `src/common/CMakeLists.txt`, add the `.cpp` and `.h` under `add_library`. Search
   for `Delay` as an example and keep the entries alphabetical.

The configuration file reloads automatically when you compile and run the plugin, though
a full build may be required.
