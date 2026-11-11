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
#ifndef SURGE_SRC_SURGE_XT_GUI_MODULATIONGRIDCONFIGURATION_H
#define SURGE_SRC_SURGE_XT_GUI_MODULATIONGRIDCONFIGURATION_H

#include "ModulationSource.h"

#include <unordered_map>

namespace Surge
{
namespace GUI
{
struct ModulationGrid : juce::DeletedAtShutdown
{
    ModulationGrid()
    {
        // Negative coordinates keep non-panel sources addressable without displaying them.
        for (int i = 0; i < n_modsources; ++i)
            add(static_cast<modsources>(i), -1, -1);

        for (int i = ms_ctrl1; i <= ms_ctrl4; ++i)
        {
            add((modsources)i, (int)i - ms_ctrl1, 0);
        }

        add(ms_ampeg, 0, 3);
        add(ms_filtereg, 1, 3);
        for (int i = ms_lfo1; i <= ms_lfo2; ++i)
        {
            add((modsources)i, (int)i - ms_lfo1 + 2, 3);
        }
        for (int i = ms_slfo1; i <= ms_slfo2; ++i)
        {
            add((modsources)i, (int)i - ms_slfo1 + 4, 3);
        }

        add(ms_random_bipolar, 0, 6, {ms_random_unipolar});
        add(ms_velocity, 1, 6);
        add(ms_keytrack, 2, 6);
        add(ms_polyaftertouch, 3, 6);
        add(ms_expression, 4, 6);
        add(ms_modwheel, 5, 6);
    }

    // This is only called at dll shutdown basically
    ~ModulationGrid() { grid = nullptr; }

    void add(modsources ms, int x, int y) { data[ms] = Entry{x, y, ms}; }
    void add(modsources ms, int x, int y, const std::vector<modsources> &alternates)
    {
        data[ms] = Entry{x, y, ms, alternates};
        for (auto a : alternates)
        {
            data[a] = Entry{a, ms};
        }
    }
    struct Entry
    {
        Entry() {}
        Entry(int x, int y, modsources ms) : x(x), y(y), ms(ms) {}
        Entry(int x, int y, modsources ms, const std::vector<modsources> &alternates)
            : x(x), y(y), ms(ms), alternates(alternates)
        {
        }
        Entry(modsources sub, modsources primary)
            : ms(sub), associatedPrimary(primary), isPrimary(false)
        {
        }

        modsources ms{ms_original};
        modsources associatedPrimary{ms_original};
        int x{-1}, y{-1};
        bool isPrimary{true};
        std::vector<modsources> alternates;
    };

    std::unordered_map<modsources, Entry> data;

    const Entry &get(modsources ms) const
    {
        jassert(data.find(ms) != data.end());
        return data.at(ms);
    }

    static ModulationGrid *grid;
    static inline const ModulationGrid *getModulationGrid()
    {
        // The juce::DeletedAtShutdown makes sure this gets cleared up
        if (!grid)
            grid = new ModulationGrid();
        return grid;
    }
};
} // namespace GUI
} // namespace Surge

#endif // SURGE_XT_MODULATIONGRIDCONFIGURATION_H
