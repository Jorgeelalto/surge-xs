/*
 * Surge XT - a free and open source hybrid synthesizer,
 * built by Surge Synth Team
 *
 * Learn more at https://surge-synthesizer.github.io/
 *
 * Copyright 2018-2024, various authors, as described in the GitHub
 * transaction log.
 *
 * Surge XT is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Surge was a commercial product from 2004-2018, copyright and ownership
 * held by Claes Johanson at Vember Audio during that period.
 * Claes made Surge open source in September 2018.
 *
 * All source for Surge XT is available at
 * https://github.com/surge-synthesizer/surge
 */

#ifndef SURGE_SRC_SURGE_XT_GUI_WIDGETS_EFFECTCHOOSER_H
#define SURGE_SRC_SURGE_XT_GUI_WIDGETS_EFFECTCHOOSER_H

#include "SurgeStorage.h"
#include "SkinSupport.h"
#include "WidgetBaseMixin.h"
#include "SurgeJUCEHelpers.h"

#include "juce_gui_basics/juce_gui_basics.h"

#include <array>

class SurgeStorage;

namespace Surge
{
namespace Widgets
{
struct EffectChooser : public juce::Component, public WidgetBaseMixin<EffectChooser>
{
    EffectChooser();
    ~EffectChooser();
    void paint(juce::Graphics &g) override;

    /*
     * These aren't used but are part of the interface. The callback casts.
     */
    float getValue() const override { return value; }
    void setValue(float f) override { value = f; }
    float value{0.f};

    void setCurrentEffect(int c) { currentEffect = c; }
    int getCurrentEffect() const { return currentEffect; }
    int currentEffect{fxslot_global1};

    void setEffectType(int index, int type);
    std::array<int, n_fx_slots> fxTypes;

    void mouseDown(const juce::MouseEvent &event) override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseEnter(const juce::MouseEvent &event) override
    {
        isHovered = true;
        mouseMove(event);
    }
    void mouseExit(const juce::MouseEvent &event) override { isHovered = false; }
    void endHover() override
    {
        if (stuckHover)
            return;

        isHovered = false;
        repaint();
    }

    bool isCurrentlyHovered() override { return isHovered; }

    void resized() override;

    // State for mouse events
    bool isHovered{false};
    int currentHover{-1}, currentClicked{-1};

    void setBypass(int b) { bypassState = b; }
    int bypassState{0};
    void setDeactivatedBitmask(int d) { deactivatedBitmask = d; };
    int getDeactivatedBitmask() const { return deactivatedBitmask; }
    int deactivatedBitmask{0};
    void toggleSelectedDeactivation();
    void setEffectSlotDeactivation(int slotIdx, bool state);

    juce::Rectangle<int> getEffectRectangle(int fx);

    void getColorsForSlot(int fxslot, juce::Colour &bgcol, juce::Colour &frcol,
                          juce::Colour &txtcol);

    SurgeStorage *storage{nullptr};
    void setStorage(SurgeStorage *s) { storage = s; }

    std::array<std::unique_ptr<juce::Component>, n_fx_slots> slotAccOverlays;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectChooser);
};

} // namespace Widgets
} // namespace Surge

#endif // SURGE_XT_EFFECTCHOOSER_H
