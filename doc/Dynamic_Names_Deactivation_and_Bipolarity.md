# Dynamic names, deactivation, and bipolarity

Surge 1.9 added dynamic parameter names, bipolarity, and activation status. These let the
UI update labels and states at draw time, without rebuilding the UI. Because `Parameter`
cannot own complex members such as `std::function`, handlers are stored in objects
managed separately from parameters.

## General

1. Implement small static function objects for names, booleans, and related values.
2. Keep them fast: they may be called often during rendering.
3. Store them outside the `Parameter` lifecycle, usually as pointers to static objects.

## Dynamic names

The parameter name is the label shown by its slider. To make it dynamic:

1. Add the control type to `Parameter::supportsDynamicName` in `Parameter.cpp`.
2. After setting up the parameter's control type, assign its `dynamicName` pointer to a
   `ParameterDynamicName` implementation. The parameter does not own this object, so a
   small stateless static handler is a common choice.
3. If a menu change needs a redraw, add the control type to
   `Parameter::is_nonlocal_on_change`. This redraws without rebuilding the UI.

`EuroTwist.cpp` shows how to derive names from the engine type, find the related
oscillator through control groups, and use the parameter index and oscillator mode.
`SurgePatch.cpp` sets up the `lfoPhaseName` handler for LFO `start_phase` parameters,
changing the label between Phase and Shuffle according to the LFO type.

## Dynamic activation

Surge 1.7 added deactivated sliders; Surge 1.9 added clusters. There are two patterns:

- **Follow deactivation:** When parameter A is deactivated, parameter B is too. The
  cluster has a menu to activate the item. See LPG Response and LPG Decay in
  `EuroTwist.cpp`.
- **Follow value:** Another value determines whether the parameter is deactivated. The
  driving value controls it, so no activation menu is needed.

### Follow deactivation pattern

The primary parameter owns the deactivation status queried by DSP through `p.deactivated`.
Its control type participates in `can_deactivate`.

Implement `ParameterDynamicDeactivationFunction` with:

- `getValue()`, which returns the parent's deactivation state.
- `getPrimaryActivationDriver()`, which returns the parent parameter or null if there
  is no parent.

The LPG parameters in `EuroTwist.cpp` demonstrate this pattern.

### Follow value pattern

Implement only `getValue()` using the relevant value. The LFO Rate and Phase parameters
use this pattern for Envelope LFOs in `SurgePatch.cpp`. Do not advertise a leader
parameter: there is no leader, or it is not toggled by setting `deactivated`.

## Dynamic bipolarity

Dynamic bipolarity changes both the UI control and value formatting. In Surge 1.9, it
supports only percent-based units with a specific control type. It changes display only:
the internal `val.f` remains in the `+/-1` range, which DSP must account for when
clamping or modeling.

Use `ct_percent_bipolar_w_dynamic_unipolar_formatting` and set `dynamicBipolar` to a
non-owned object implementing `ParameterDynamicBool::getValue()`. When false, the value
is formatted as unipolar (0% to 100% instead of -100% to 100%) and the slider background
changes, while the stored value remains -1 to 1.

`EuroTwist.cpp` demonstrates this with the `kScaleBasedOnIsBiPolar` flag on `displayinfo`.
