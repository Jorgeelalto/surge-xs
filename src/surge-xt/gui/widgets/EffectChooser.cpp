/*
 * Surge XT - a free and open source hybrid synthesizer,
 * built by Surge Synth Team
 *
 * Copyright 2018-2024, various authors, as described in the GitHub
 * transaction log.
 *
 * Surge XT is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 */
#include "EffectChooser.h"
#include "AccessibleHelpers.h"
#include "RuntimeFont.h"

#include <algorithm>

namespace
{
const char *effectName(int slot)
{
    const auto type = fixedFxTypeForSlot(slot);
    return fx_type_names[type];
}
} // namespace

namespace Surge::Widgets
{

EffectChooser::EffectChooser() : juce::Component(), WidgetBaseMixin<EffectChooser>(this)
{
    fxTypes.fill(fxt_off);
    setRepaintsOnMouseActivity(true);
    setAccessible(true);
    setFocusContainerType(juce::Component::FocusContainerType::focusContainer);

    for (size_t i = 0; i < fixedFxSlots.size(); ++i)
    {
        const auto slot = fixedFxSlots[i];
        auto button = std::make_unique<OverlayAsAccessibleButton<EffectChooser>>(
            this, effectName(slot));
        button->onPress = [this, slot](auto *) {
            currentClicked = currentEffect = slot;
            notifyValueChanged();
        };
        button->onReturnKey = [this, slot](auto *) {
            currentClicked = currentEffect = slot;
            notifyValueChanged();
            return true;
        };
        button->onGetIsChecked = [this, slot](auto *) { return currentEffect == slot; };
        addAndMakeVisible(*button);
        slotAccOverlays[i] = std::move(button);
    }
}

EffectChooser::~EffectChooser() = default;

void EffectChooser::paint(juce::Graphics &g)
{
    g.setFont(skin->fontManager->getLatoAtSize(9));
    for (auto slot : fixedFxSlots)
    {
        juce::Colour background, border, text;
        getColorsForSlot(slot, background, border, text);
        auto bounds = getEffectRectangle(slot);
        g.setColour(background);
        g.fillRect(bounds);
        g.setColour(border);
        g.drawRect(bounds);
        g.setColour(text);
        g.drawText(effectName(slot), bounds, juce::Justification::centred);
    }
}

void EffectChooser::resized()
{
    for (size_t i = 0; i < fixedFxSlots.size(); ++i)
        slotAccOverlays[i]->setBounds(getEffectRectangle(fixedFxSlots[i]));
}

juce::Rectangle<int> EffectChooser::getEffectRectangle(int slot)
{
    const int count = static_cast<int>(fixedFxSlots.size());
    const int gap = 2;
    const int available = std::max(0, getHeight() - gap * (count - 1));
    for (size_t i = 0; i < fixedFxSlots.size(); ++i)
        if (fixedFxSlots[i] == slot)
        {
            const int index = static_cast<int>(i);
            const int top = available * index / count + index * gap;
            const int bottom = available * (index + 1) / count + index * gap;
            return {0, top, getWidth(), bottom - top};
        }
    return {};
}

void EffectChooser::mouseDown(const juce::MouseEvent &event)
{
    if (forwardedMainFrameMouseDowns(event) || event.mods.isPopupMenu())
        return;
    for (auto slot : fixedFxSlots)
        if (getEffectRectangle(slot).contains(event.getPosition()))
        {
            currentClicked = currentEffect = slot;
            notifyValueChanged();
            repaint();
            return;
        }
}

void EffectChooser::mouseMove(const juce::MouseEvent &event)
{
    int nextHover = -1;
    for (auto slot : fixedFxSlots)
        if (getEffectRectangle(slot).contains(event.getPosition()))
            nextHover = slot;
    if (nextHover != currentHover)
    {
        currentHover = nextHover;
        repaint();
    }
}

void EffectChooser::getColorsForSlot(int slot, juce::Colour &background, juce::Colour &border,
                                     juce::Colour &text)
{
    const bool hovered = isHovered && slot == currentHover;
    if (slot == currentEffect)
    {
        background = skin->getColor(hovered ? Colors::Effect::Grid::Selected::BackgroundHover
                                            : Colors::Effect::Grid::Selected::Background);
        border = skin->getColor(hovered ? Colors::Effect::Grid::Selected::BorderHover
                                        : Colors::Effect::Grid::Selected::Border);
        text = skin->getColor(hovered ? Colors::Effect::Grid::Selected::TextHover
                                      : Colors::Effect::Grid::Selected::Text);
    }
    else
    {
        background = skin->getColor(hovered ? Colors::Effect::Grid::Unselected::BackgroundHover
                                            : Colors::Effect::Grid::Unselected::Background);
        border = skin->getColor(hovered ? Colors::Effect::Grid::Unselected::BorderHover
                                        : Colors::Effect::Grid::Unselected::Border);
        text = skin->getColor(hovered ? Colors::Effect::Grid::Unselected::TextHover
                                      : Colors::Effect::Grid::Unselected::Text);
    }
}

void EffectChooser::setEffectType(int index, int type) { fxTypes[index] = type; }

void EffectChooser::toggleSelectedDeactivation()
{
    storage->reportError("Effects in the fixed chain cannot be disabled.", "Fixed FX Chain");
}

void EffectChooser::setEffectSlotDeactivation(int, bool state)
{
    if (state)
        storage->reportError("Effects in the fixed chain cannot be disabled.", "Fixed FX Chain");
}

template <> struct DiscreteAHRange<EffectChooser>
{
    static int iMaxV(EffectChooser *) { return static_cast<int>(fixedFxSlots.size()) - 1; }
    static int iMinV(EffectChooser *) { return 0; }
};

template <> struct DiscreteAHStringValue<EffectChooser>
{
    static std::string stringValue(EffectChooser *comp, double)
    {
        return effectName(comp->getCurrentEffect());
    }
};

template <> struct DiscreteRO<EffectChooser>
{
    static bool isReadOnly(EffectChooser *) { return true; }
};

std::unique_ptr<juce::AccessibilityHandler> EffectChooser::createAccessibilityHandler()
{
    return std::make_unique<DiscreteAH<EffectChooser, juce::AccessibilityRole::group>>(this);
}
} // namespace Surge::Widgets
