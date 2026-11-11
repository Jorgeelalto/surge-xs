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

#ifndef SURGE_SRC_COMMON_FILTERCONFIGURATION_H
#define SURGE_SRC_COMMON_FILTERCONFIGURATION_H

#include "Parameter.h"
#include "sst/filters.h"
#include "sst/waveshapers.h"

#include <array>

inline int filterSubtypeCount(int filterType)
{
    using sst::filters::FilterType;

    if (filterType < 0 || filterType >= FilterType::num_filter_types)
        return 0;

    switch (filterType)
    {
    case FilterType::fut_bp12:
        return 3;
    case FilterType::fut_obxd_2pole_lp:
    case FilterType::fut_diode:
    case FilterType::fut_k35_lp:
    case FilterType::fut_hp24:
        return sst::filters::fut_subcount[filterType];
    case FilterType::fut_none:
    default:
        return 0;
    }
}

struct FilterSelectorMapper : public ParameterDiscreteIndexRemapper
{
    std::vector<int> displayOrder;
    std::unordered_map<int, int> displayIndex;
    FilterSelectorMapper()
    {
        using sst::filters::FilterType;

        displayOrder = {FilterType::fut_none,       FilterType::fut_obxd_2pole_lp,
                        FilterType::fut_diode,      FilterType::fut_k35_lp,
                        FilterType::fut_bp12,       FilterType::fut_hp24};
        int index = 0;
        for (const auto type : displayOrder)
            displayIndex[type] = index++;
    }

    virtual int remapStreamedIndexToDisplayIndex(int i) const override
    {
        auto it = displayIndex.find(i);
        return it == displayIndex.end() ? -1 : it->second;
    }
    virtual std::string nameAtStreamedIndex(int i) const override
    {
        return sst::filters::filter_menu_names[i];
    }
    virtual bool hasGroupNames() const override { return false; };

    virtual std::string groupNameAtStreamedIndex(int) const override
    {
        return {};
    }

    virtual bool sortGroupNames() const override { return false; }
    bool useRemappedOrderingForGroupsIfNotSorted() const override { return true; }

    virtual bool supportsTotalIndexOrdering() const override { return true; }
    virtual const std::vector<int> totalIndexOrdering() const override
    {
        return displayOrder;
    }
};

/*
 * Finally we need to map streaming indices to positions on the glyph display. This
 * should *really* be in UI code but it is just a declaration and having all the declarations
 * together is useful. In the far distant future perhaps we customize this by skin.
 */

const int lprow = 1;
const int bprow = 2;
const int hprow = 3;

const int fut_glyph_index[sst::filters::num_filter_types][2] = {
    {0, 0},      // Off
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {1, hprow},  // HP 24 dB
    {0, bprow},  // BP 12 dB
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {6, lprow},  // LP Oberheim
    {0, 0},
    {2, lprow},  // LP K35
    {0, 0},
    {5, lprow},  // LP Diode
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}
};

inline constexpr std::array<int, 3> availableWaveshaperTypes = {
    static_cast<int>(sst::waveshapers::WaveshaperType::wst_soft),
    static_cast<int>(sst::waveshapers::WaveshaperType::wst_asym),
    static_cast<int>(sst::waveshapers::WaveshaperType::wst_ojd)};

inline bool isAvailableWaveshaperType(int type)
{
    for (auto available : availableWaveshaperTypes)
        if (type == available)
            return true;
    return false;
}

inline sst::waveshapers::QuadWaveshaperPtr getAvailableWaveshaper(int type)
{
    using namespace sst::waveshapers;
    switch (static_cast<WaveshaperType>(type))
    {
    case WaveshaperType::wst_soft:
        return TANH;
    case WaveshaperType::wst_asym:
        return ASYM_SSE2;
    case WaveshaperType::wst_ojd:
        return OJD;
    default:
        return nullptr;
    }
}

struct WaveShaperSelectorMapper : public ParameterDiscreteIndexRemapper
{
    int remapStreamedIndexToDisplayIndex(int i) const override
    {
        for (std::size_t index = 0; index < availableWaveshaperTypes.size(); ++index)
            if (availableWaveshaperTypes[index] == i)
                return static_cast<int>(index);
        return -1;
    }
    std::string nameAtStreamedIndex(int i) const override
    {
        return sst::waveshapers::wst_names[i];
    }
    bool hasGroupNames() const override { return false; }
    std::string groupNameAtStreamedIndex(int) const override { return {}; }
    bool sortGroupNames() const override { return false; }
    bool useRemappedOrderingForGroupsIfNotSorted() const override { return true; }
    bool supportsTotalIndexOrdering() const override { return true; }
    const std::vector<int> totalIndexOrdering() const override
    {
        return {availableWaveshaperTypes.begin(), availableWaveshaperTypes.end()};
    }
};
#endif // SURGE_SRC_COMMON_FILTERCONFIGURATION_H
