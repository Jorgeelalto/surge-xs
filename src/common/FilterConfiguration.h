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

const char wst_ui_names[(int)sst::waveshapers::WaveshaperType::n_ws_types][16] = {
    "Off",     "Soft",     "Hard",     "Asym",     "Sine",    "Digital",  "Harm 2",  "Harm 3",
    "Harm 4",  "Harm 5",   "FullRect", "HalfPos",  "HalfNeg", "SoftRect", "1Fold",   "2Fold",
    "WCFold",  "Add12",    "Add13",    "Add14",    "Add15",   "Add1-5",   "AddSaw3", "AddSqr3",
    "Fuzz",    "SoftFz",   "HeavyFz",  "CenterFz", "EdgeFz",  "Sin+x",    "Sin2x+x", "Sin3x+x",
    "Sin7x+x", "Sin10x+x", "2Cycle",   "7Cycle",   "10Cycle", "2CycleB",  "7CycleB", "10CycleB",
    "Medium",  "OJD",      "Sft1Fld"};

// Subset used in distortion and rotary
static constexpr int n_fxws = 8;
static constexpr std::array<sst::waveshapers::WaveshaperType, n_fxws> FXWaveShapers = {
    sst::waveshapers::WaveshaperType::wst_soft, sst::waveshapers::WaveshaperType::wst_hard,
    sst::waveshapers::WaveshaperType::wst_asym, sst::waveshapers::WaveshaperType::wst_sine,
    sst::waveshapers::WaveshaperType::wst_digital, // If you adjust this list above here, you
                                                   // break 1.9 patch compat
    sst::waveshapers::WaveshaperType::wst_ojd, sst::waveshapers::WaveshaperType::wst_fwrectify,
    sst::waveshapers::WaveshaperType::wst_fuzzsoft};

struct WaveShaperSelectorMapper : public ParameterDiscreteIndexRemapper
{
    std::vector<std::pair<int, std::string>> mapping;
    std::unordered_map<int, int> inverseMapping;
    void p(sst::waveshapers::WaveshaperType i, const std::string &s)
    {
        mapping.emplace_back((int)i, s);
    }

    WaveShaperSelectorMapper()
    {
        // Obviously improve this
        p(sst::waveshapers::WaveshaperType::wst_none, "");

        p(sst::waveshapers::WaveshaperType::wst_soft, "Saturator");
        p(sst::waveshapers::WaveshaperType::wst_zamsat, "Saturator");
        p(sst::waveshapers::WaveshaperType::wst_hard, "Saturator");
        p(sst::waveshapers::WaveshaperType::wst_asym, "Saturator");
        p(sst::waveshapers::WaveshaperType::wst_ojd, "Saturator");

        p(sst::waveshapers::WaveshaperType::wst_sine, "Effect");
        p(sst::waveshapers::WaveshaperType::wst_digital, "Effect");

        p(sst::waveshapers::WaveshaperType::wst_cheby2, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_cheby3, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_cheby4, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_cheby5, "Harmonic");

        p(sst::waveshapers::WaveshaperType::wst_add12, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_add13, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_add14, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_add15, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_add12345, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_addsaw3, "Harmonic");
        p(sst::waveshapers::WaveshaperType::wst_addsqr3, "Harmonic");

        p(sst::waveshapers::WaveshaperType::wst_fwrectify, "Rectifiers");
        p(sst::waveshapers::WaveshaperType::wst_poswav, "Rectifiers");
        p(sst::waveshapers::WaveshaperType::wst_negwav, "Rectifiers");
        p(sst::waveshapers::WaveshaperType::wst_softrect, "Rectifiers");

        p(sst::waveshapers::WaveshaperType::wst_softfold, "Wavefolder");
        p(sst::waveshapers::WaveshaperType::wst_singlefold, "Wavefolder");
        p(sst::waveshapers::WaveshaperType::wst_dualfold, "Wavefolder");
        p(sst::waveshapers::WaveshaperType::wst_westfold, "Wavefolder");

        p(sst::waveshapers::WaveshaperType::wst_fuzz, "Fuzz");
        p(sst::waveshapers::WaveshaperType::wst_fuzzheavy, "Fuzz");
        p(sst::waveshapers::WaveshaperType::wst_fuzzctr, "Fuzz");
        p(sst::waveshapers::WaveshaperType::wst_fuzzsoft, "Fuzz");
        p(sst::waveshapers::WaveshaperType::wst_fuzzsoftedge, "Fuzz");

        p(sst::waveshapers::WaveshaperType::wst_sinpx, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_sin2xpb, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_sin3xpb, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_sin7xpb, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_sin10xpb, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_2cyc, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_7cyc, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_10cyc, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_2cycbound, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_7cycbound, "Trigonometric");
        p(sst::waveshapers::WaveshaperType::wst_10cycbound, "Trigonometric");

        int c = 0;
        for (auto e : mapping)
            inverseMapping[e.first] = c++;

        if (mapping.size() != (int)sst::waveshapers::WaveshaperType::n_ws_types)
            std::cout << "BAD MAPPING TYPES" << std::endl;
    }

    int remapStreamedIndexToDisplayIndex(int i) const override { return inverseMapping.at(i); }
    std::string nameAtStreamedIndex(int i) const override { return sst::waveshapers::wst_names[i]; }
    bool hasGroupNames() const override { return true; };

    std::string groupNameAtStreamedIndex(int i) const override
    {
        return mapping[inverseMapping.at(i)].second;
    }

    bool sortGroupNames() const override { return false; }
    bool useRemappedOrderingForGroupsIfNotSorted() const override { return true; }

    bool supportsTotalIndexOrdering() const override { return true; }
    const std::vector<int> totalIndexOrdering() const override
    {
        auto res = std::vector<int>();
        for (auto m : mapping)
            res.push_back(m.first);
        return res;
    }
};
#endif // SURGE_SRC_COMMON_FILTERCONFIGURATION_H
