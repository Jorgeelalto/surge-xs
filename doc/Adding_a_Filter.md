# Adding a filter

1. Create `src/common/dsp/filters/MyFilter.h` and `MyFilter.cpp`. Use `VintageLadders.cpp`
   as a reference. Implement the free functions `makeCoefficients()` and `process()`.
   `process()` has a fixed signature; `makeCoefficients()` can use your chosen signature.
   The Vintage Ladders functions are good starting points. Add the `.cpp` to
   `src/common/CMakeLists.txt` beside `VintageLadders.cpp`.

2. Register the filter in `FilterConfiguration.h`:
   - Add its enum value to `fu_type` **at the end**.
   - Add its names to `fut_names` and `fut_menu_names`.
   - Add its subtype count to `fut_subcount`, if it has subtypes.
   - Add any subtype strings or constants.
   - Add it to `FilterSelectorMapper` and `fut_glyph_index`.

3. In `FilterCoefficientMaker.h`, configure the filter's state and call your namespaced
   `makeCoefficients()` from the `FilterCoefficientMaker::MakeCoeffs()` switch. You can
   use the eight values in `C[8]`. Coefficients are updated at most once per
   `BLOCK_SIZE_OS`, and only when `F` or `R` changes.

4. In `QuadFilterUnit.cpp`, add your `process()` function to `GetQFPtrFilterUnit()`.
   It must have this signature:

   ```cpp
   _m128 op(QuadFilterUnitState * __restrict, _m128 in)
   ```

   The function receives a `QuadFilterUnitState` containing the coefficients in `C[8]`
   and registers in `R`. Processing runs in parallel per voice, with 1–4 SSE channels
   populated. Use `f->active` to identify active voices. The `in`, `C`, and `R` values
   are also SSE-wide arrays. See the existing SSE implementations and the manual unroll
   in `NonlinearFeedback.cpp`.

5. If the filter has subtypes, add their display strings in `Parameter.cpp`. See Vintage
   Ladders for an example.

6. If the SSE function needs the subtype, check the switches in `SurgeVoice::SetQFB()`.

You can use Vintage Ladders as a worked example.
