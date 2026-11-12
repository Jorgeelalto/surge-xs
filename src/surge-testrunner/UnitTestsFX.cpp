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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>

#include "HeadlessUtils.h"
#include "Player.h"

#include "SurgeStorage.h"
#include "catch2/catch_amalgamated.hpp"

#include "UnitTestUtilities.h"
#include "DelayEffect.h"
#include "Reverb1Effect.h"
#include "BBDEnsembleEffect.h"
#include "ConditionerEffect.h"
#include "dsp/FixedFxSceneDelay.h"

using namespace Surge::Test;

TEST_CASE("Fixed FX chain initializes and stays dry", "[fx][fixed-fx]")
{
    auto surge = Surge::Headless::createSurge(44100);
    REQUIRE(surge);
    surge->process();
    auto &patch = surge->storage.getPatch();

    for (int slot = 0; slot < n_fx_slots; ++slot)
    {
        REQUIRE(patch.fx[slot].type.val.i == fixedFxTypeForSlot(slot));
#if defined(_M_ARM64EC)
        const bool available = fixedFxTypeForSlot(slot) != fxt_off && slot != fxslot_global1;
#else
        const bool available = fixedFxTypeForSlot(slot) != fxt_off;
#endif
        REQUIRE(static_cast<bool>(surge->fx[slot]) == available);
    }

#if !defined(_M_ARM64EC)
    REQUIRE(patch.fx[fxslot_global1].p[BBDEnsembleEffect::ens_mix].val.f == 0.f);
#endif
    REQUIRE(patch.fx[fxslot_global2].p[DelayEffect::dly_mix].val.f == 0.f);
    REQUIRE(patch.fx[fxslot_global3].p[Reverb1Effect::rev1_mix].val.f == 0.f);
    REQUIRE(patch.fx[fxslot_global4].p[ConditionerEffect::cond_mix].val.f == 0.f);

    for (auto slot : fixedFxSlots)
    {
        if (!surge->fx[slot])
            continue;
        if (slot == fxslot_global4)
            continue;
        float left alignas(16)[BLOCK_SIZE], right alignas(16)[BLOCK_SIZE];
        std::fill(left, left + BLOCK_SIZE, 0.25f);
        std::fill(right, right + BLOCK_SIZE, -0.5f);
        surge->fx[slot]->process(left, right);
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            REQUIRE(left[i] == Approx(0.25f).margin(1.e-5f));
            REQUIRE(right[i] == Approx(-0.5f).margin(1.e-5f));
        }
    }

    for (int slot = 0; slot < n_fx_slots; ++slot)
    {
        auto &type = patch.fx[slot].type;
        surge->setParameter01(surge->idForParameter(&type), 1.f, false);
        REQUIRE(type.val.i == fixedFxTypeForSlot(slot));
    }
}

TEST_CASE("Fixed FX chain processes in order after the scene mix", "[fx][fixed-fx]")
{
    struct Probe : Effect
    {
        Probe(SurgeStorage *storage, FxStorage *data, pdata *pd, int slot, std::vector<int> &calls,
              std::vector<float> &inputs, std::vector<bool> &activeStates)
            : Effect(storage, data, pd), slot(slot), calls(calls), inputs(inputs),
              activeStates(activeStates)
        {
        }
        bool process_ringout(float *left, float *right, bool active) override
        {
            calls.push_back(slot);
            inputs.push_back(left[0]);
            activeStates.push_back(active);
            for (int i = 0; i < BLOCK_SIZE; ++i)
            {
                left[i] += 0.125f;
                right[i] += 0.125f;
            }
            return true;
        }
        int slot;
        std::vector<int> &calls;
        std::vector<float> &inputs;
        std::vector<bool> &activeStates;
    };

    auto surge = Surge::Headless::createSurge(44100);
    REQUIRE(surge);
    surge->process();
    auto &patch = surge->storage.getPatch();
    patch.scenemode.val.i = sm_dual;
    surge->playNote(0, 60, 100, 0, -1);
    for (int i = 0; i < 20; ++i)
        surge->process();
    std::vector<int> calls;
    std::vector<float> inputs;
    std::vector<bool> activeStates;
    auto sceneA = surge->storage.scenesOutputData.getSceneData(0, 0);
    auto sceneB = surge->storage.scenesOutputData.getSceneData(1, 0);
    surge->storage.sceneHardclipMode[0] = SurgeStorage::BYPASS_HARDCLIP;
    surge->storage.sceneHardclipMode[1] = SurgeStorage::BYPASS_HARDCLIP;
    for (int slot = 0; slot < n_fx_slots; ++slot)
        surge->fx[slot] = std::make_unique<Probe>(&surge->storage, &patch.fx[slot],
                                                  patch.globaldata, slot, calls, inputs, activeStates);
    patch.fx_bypass.val.i = fxb_no_fx;
    patch.fx_disable.val.i = -1;
    surge->process();
    REQUIRE(calls == std::vector<int>(fixedFxSlots.begin(), fixedFxSlots.end()));
    REQUIRE(inputs[0] == Approx(sceneA.get()[0] + sceneB.get()[0]).margin(1.e-6f));
    REQUIRE(inputs[1] == Approx(inputs[0] + 0.125f));
    REQUIRE(inputs[2] == Approx(inputs[1] + 0.125f));
    REQUIRE(inputs[3] == Approx(inputs[2] + 0.125f));
    REQUIRE(activeStates[1]);
    REQUIRE(activeStates[2]);
    REQUIRE(activeStates[3]);
}

TEST_CASE("Conditioner delays dry and wet signals equally", "[fx][fixed-fx]")
{
    auto surge = Surge::Headless::createSurge(44100);
    REQUIRE(surge);
    surge->process();
    auto &patch = surge->storage.getPatch();
    auto *effect = surge->fx[fxslot_global4].get();
    REQUIRE(effect);

    for (float wet : {0.f, 1.f})
    {
        patch.fx[fxslot_global4].p[ConditionerEffect::cond_mix].val.f = wet;
        patch.copy_globaldata(patch.globaldata);
        effect->init();
        float left alignas(16)[BLOCK_SIZE], right alignas(16)[BLOCK_SIZE];
        for (int block = 0; block < 10; ++block)
        {
            std::fill(left, left + BLOCK_SIZE, 0.f);
            std::fill(right, right + BLOCK_SIZE, 0.f);
            effect->process(left, right);
        }

        for (int sample = 0; sample < fixedFxLatencySamples + BLOCK_SIZE; sample += BLOCK_SIZE)
        {
            std::fill(left, left + BLOCK_SIZE, 0.f);
            std::fill(right, right + BLOCK_SIZE, 0.f);
            if (sample == 0)
            {
                left[0] = 0.25f;
                right[0] = -0.5f;
            }
            effect->process(left, right);
            for (int i = 0; i < BLOCK_SIZE; ++i)
            {
                if (sample + i == fixedFxLatencySamples)
                {
                    if (wet == 0.f)
                    {
                        REQUIRE(left[i] == Approx(0.25f).margin(1.e-6f));
                        REQUIRE(right[i] == Approx(-0.5f).margin(1.e-6f));
                    }
                    else
                    {
                        REQUIRE(std::abs(left[i]) > 0.01f);
                        REQUIRE(std::abs(right[i]) > 0.01f);
                    }
                }
                else if (sample + i < fixedFxLatencySamples)
                {
                    REQUIRE(left[i] == Approx(0.f).margin(1.e-6f));
                    REQUIRE(right[i] == Approx(0.f).margin(1.e-6f));
                }
            }
        }
    }
}

TEST_CASE("Conditioner publishes limiter gain reduction", "[fx][fixed-fx]")
{
    auto surge = Surge::Headless::createSurge(44100);
    REQUIRE(surge);
    surge->process();
    auto *effect = dynamic_cast<ConditionerEffect *>(surge->fx[fxslot_global4].get());
    REQUIRE(effect);
    auto &patch = surge->storage.getPatch();
    const auto &threshold = patch.fx[fxslot_global4].p[ConditionerEffect::cond_threshold];
    patch.globaldata[threshold.id].f = -30.f;
    effect->init();
    REQUIRE(effect->gainReduction.load(std::memory_order_relaxed) == 1.f);

    float left alignas(16)[BLOCK_SIZE], right alignas(16)[BLOCK_SIZE];
    for (int block = 0; block < 128; ++block)
    {
        std::fill(left, left + BLOCK_SIZE, 0.9f);
        std::fill(right, right + BLOCK_SIZE, 0.9f);
        effect->process(left, right);
    }
    const auto reduced = effect->gainReduction.load(std::memory_order_relaxed);
    REQUIRE(reduced > 0.f);
    REQUIRE(reduced < 0.8f);

    for (int block = 0; block < 128; ++block)
        effect->process_only_control();
    REQUIRE(effect->gainReduction.load(std::memory_order_relaxed) > reduced);
    effect->suspend();
    REQUIRE(effect->gainReduction.load(std::memory_order_relaxed) == 1.f);
}

TEST_CASE("Scene output delay stays aligned and resets", "[fx][fixed-fx]")
{
    FixedFxSceneDelay delay;
    float input[2][2]{}, output[2][2]{};
    for (int sample = 0; sample < fixedFxLatencySamples + 1; ++sample)
    {
        for (int scene = 0; scene < 2; ++scene)
            for (int channel = 0; channel < 2; ++channel)
                input[scene][channel] = sample == 0 ? 1.f + scene * 2 + channel : 0.f;
        delay.process(input, output);
        for (int scene = 0; scene < 2; ++scene)
            for (int channel = 0; channel < 2; ++channel)
                REQUIRE(output[scene][channel] ==
                        (sample == fixedFxLatencySamples ? 1.f + scene * 2 + channel : 0.f));
    }

    input[0][0] = 1.f;
    delay.process(input, output);
    delay.reset();
    input[0][0] = 0.f;
    for (int sample = 0; sample <= fixedFxLatencySamples; ++sample)
    {
        delay.process(input, output);
        REQUIRE(output[0][0] == 0.f);
    }
}

TEST_CASE("Conditioner smooths Mix changes without changing latency", "[fx][fixed-fx]")
{
    auto surge = Surge::Headless::createSurge(44100);
    REQUIRE(surge);
    surge->process();
    auto &patch = surge->storage.getPatch();
    auto *effect = surge->fx[fxslot_global4].get();
    REQUIRE(effect);
    float left alignas(16)[BLOCK_SIZE], right alignas(16)[BLOCK_SIZE];

    for (int block = 0; block < 20; ++block)
    {
        std::fill(left, left + BLOCK_SIZE, 0.2f);
        std::fill(right, right + BLOCK_SIZE, 0.2f);
        effect->process(left, right);
    }
    REQUIRE(left[0] == Approx(0.2f).margin(1.e-6f));

    auto &mix = patch.fx[fxslot_global4].p[ConditionerEffect::cond_mix];
    patch.globaldata[mix.id].f = 1.f;
    float first = 0.f, last = 0.f;
    for (int block = 0; block < 20; ++block)
    {
        std::fill(left, left + BLOCK_SIZE, 0.2f);
        std::fill(right, right + BLOCK_SIZE, 0.2f);
        effect->process(left, right);
        if (block == 0)
            first = left[0];
        last = left[BLOCK_SIZE - 1];
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            REQUIRE(std::isfinite(left[i]));
            REQUIRE(left[i] > 0.01f);
        }
    }
    REQUIRE(first == Approx(0.2f).margin(0.01f));
    REQUIRE(last < first - 0.01f);
}

TEST_CASE("Scenes Output Data", "[fx]")
{
    SECTION("Providing data")
    {
        Surge::Storage::ScenesOutputData scenesOutputData{};
        REQUIRE(!scenesOutputData.thereAreClients(0));
        REQUIRE(!scenesOutputData.thereAreClients(1));

        {
            auto clientDataLeft = scenesOutputData.getSceneData(0, 0);
            auto clientDataRight = scenesOutputData.getSceneData(0, 1);
            REQUIRE(clientDataLeft);
            REQUIRE(clientDataRight);
            REQUIRE(scenesOutputData.thereAreClients(0));
            REQUIRE(!scenesOutputData.thereAreClients(1));

            float data[32]{1.0f};
            scenesOutputData.provideSceneData(0, 0, data);
            scenesOutputData.provideSceneData(0, 1, data);
            REQUIRE(scenesOutputData.getSceneData(0, 0).get()[0] == 1.0f);
            REQUIRE(scenesOutputData.getSceneData(0, 1).get()[0] == 1.0f);

            scenesOutputData.provideSceneData(1, 0, data);
            scenesOutputData.provideSceneData(1, 1, data);
            REQUIRE(scenesOutputData.getSceneData(1, 0).get()[0] == 0.0f);
            REQUIRE(scenesOutputData.getSceneData(1, 1).get()[0] == 0.0f);
        }
        REQUIRE(!scenesOutputData.thereAreClients(0));
        REQUIRE(!scenesOutputData.thereAreClients(1));
    }
}

TEST_CASE("Stopping Sound Clears Poisoned FX State", "[fx]") // See issue 8240
{
    // A single non-finite sample that reaches an effect's internal state must not survive
    // stopSound(). Otherwise it recirculates forever, and the output hard clipper turns it
    // into a sustained full scale signal that no volume control can attenuate.
    for (auto slot : fixedFxSlots)
    {
#if defined(_M_ARM64EC)
        if (slot == fxslot_global1)
            continue;
#endif
        auto t = fixedFxTypeForSlot(slot);
        DYNAMIC_SECTION("FX " << t << " " << fx_type_names[t])
        {
            auto surge = Surge::Headless::createSurge(48000);
            REQUIRE(surge);

            surge->process();
            REQUIRE(surge->fx[slot]);

            float L alignas(16)[BLOCK_SIZE], R alignas(16)[BLOCK_SIZE];

            std::fill(L, L + BLOCK_SIZE, std::numeric_limits<float>::quiet_NaN());
            std::fill(R, R + BLOCK_SIZE, std::numeric_limits<float>::quiet_NaN());
            surge->fx[slot]->process(L, R);

            surge->stopSound();

            for (int b = 0; b < 100; ++b)
            {
                std::fill(L, L + BLOCK_SIZE, 0.f);
                std::fill(R, R + BLOCK_SIZE, 0.f);
                surge->fx[slot]->process(L, R);

                INFO("Block " << b);
                for (int s = 0; s < BLOCK_SIZE; ++s)
                {
                    REQUIRE(std::isfinite(L[s]));
                    REQUIRE(std::isfinite(R[s]));
                }
            }
        }
    }
}

namespace
{
struct DelayLineModeProbe
{
    int lineModeLeft{0};
    int lineModeRight{0};
    bool linkRight{false};
    float startSeconds{0.1f};
    float endSeconds{0.2f};
    float modRate{0.f};
    float modDepth{0.f};
    int channel{0};

    static constexpr int sampleRate{44100};

    std::shared_ptr<SurgeSynthesizer> makeDelay() const
    {
        auto surge = Surge::Headless::createSurge(sampleRate);
        REQUIRE(surge);

        surge->process();

        auto &patch = surge->storage.getPatch();
        auto *fx = &(patch.fx[fxslot_global2]);

        fx->p[DelayEffect::dly_time_left].val.f = std::log2(startSeconds);
        fx->p[DelayEffect::dly_time_right].val.f = std::log2(startSeconds);
        fx->p[DelayEffect::dly_time_right].deactivated = linkRight;
        fx->p[DelayEffect::dly_time_left].deform_type = lineModeLeft;
        fx->p[DelayEffect::dly_time_right].deform_type = lineModeRight;
        fx->p[DelayEffect::dly_feedback].val.f = 0.f;
        fx->p[DelayEffect::dly_crossfeed].val.f = 0.f;
        fx->p[DelayEffect::dly_mod_rate].val.f = modRate;
        fx->p[DelayEffect::dly_mod_depth].val.f = modDepth;
        fx->p[DelayEffect::dly_input_channel].val.f = 0.f;
        fx->p[DelayEffect::dly_mix].val.f = 1.f;
        fx->p[DelayEffect::dly_width].val.f = 0.f;
        fx->p[DelayEffect::dly_lowcut].deactivated = true;
        fx->p[DelayEffect::dly_highcut].deactivated = true;

        patch.copy_globaldata(patch.globaldata);
        surge->fx[fxslot_global2]->init();

        return surge;
    }

    /*
     * Render an impulse through a 100% wet, feedback free Dual Delay whose delay time
     * changes from startSeconds to endSeconds in the same block the impulse arrives in,
     * and report the sample index at which the delayed impulse comes back out.
     */
    int impulseReturnIndex() const
    {
        auto surge = makeDelay();
        auto &patch = surge->storage.getPatch();
        auto *fx = &(patch.fx[fxslot_global2]);

        float dataL alignas(16)[BLOCK_SIZE], dataR alignas(16)[BLOCK_SIZE];

        auto runBlock = [&](bool impulse) {
            std::fill(dataL, dataL + BLOCK_SIZE, 0.f);
            std::fill(dataR, dataR + BLOCK_SIZE, 0.f);

            if (impulse)
            {
                dataL[0] = 1.f;
                dataR[0] = 1.f;
            }

            surge->fx[fxslot_global2]->process(dataL, dataR);
        };

        for (int b = 0; b < 100; ++b)
        {
            runBlock(false);
        }

        fx->p[DelayEffect::dly_time_left].val.f = std::log2(endSeconds);
        fx->p[DelayEffect::dly_time_right].val.f = std::log2(endSeconds);
        patch.copy_globaldata(patch.globaldata);

        int peakIndex = -1;
        float peakValue = 0.f;

        for (int b = 0, n = 0; b < sampleRate / BLOCK_SIZE; ++b)
        {
            runBlock(b == 0);

            for (int k = 0; k < BLOCK_SIZE; ++k, ++n)
            {
                auto v = std::fabs(channel == 0 ? dataL[k] : dataR[k]);

                if (v > peakValue)
                {
                    peakValue = v;
                    peakIndex = n;
                }
            }
        }

        INFO("Delayed impulse peaked at " << peakValue);
        REQUIRE(peakValue > 0.05f);

        return peakIndex;
    }

    /*
     * Feed a steady sine through the same delay and report the largest sample to sample
     * step in the output, once while the delay time is held and once across the change
     * from startSeconds to endSeconds. A tap that is repositioned without a crossfade
     * splices two unrelated phases of the sine together, which shows up here as a step
     * far larger than the sine's own slope.
     */
    std::pair<float, float> sineStepDiscontinuity() const
    {
        auto surge = makeDelay();
        auto &patch = surge->storage.getPatch();
        auto *fx = &(patch.fx[fxslot_global2]);

        // deliberately not a whole number of cycles per delay time, so that the two taps
        // are out of phase with each other
        constexpr double twoPi{6.283185307179586};
        const double dPhase = twoPi * 437.0 / sampleRate;
        double phase = 0.0;

        float dataL alignas(16)[BLOCK_SIZE], dataR alignas(16)[BLOCK_SIZE];
        float previous = 0.f;
        bool havePrevious = false;

        auto runBlock = [&](float *maxDelta) {
            for (int k = 0; k < BLOCK_SIZE; ++k)
            {
                dataL[k] = (float)std::sin(phase);
                dataR[k] = dataL[k];
                phase += dPhase;
            }

            surge->fx[fxslot_global2]->process(dataL, dataR);

            for (int k = 0; k < BLOCK_SIZE; ++k)
            {
                if (havePrevious && maxDelta)
                {
                    *maxDelta = std::max(*maxDelta, std::fabs(dataL[k] - previous));
                }

                previous = dataL[k];
                havePrevious = true;
            }
        };

        for (int b = 0; b < 400; ++b)
        {
            runBlock(nullptr);
        }

        float held = 0.f;

        for (int b = 0; b < 200; ++b)
        {
            runBlock(&held);
        }

        fx->p[DelayEffect::dly_time_left].val.f = std::log2(endSeconds);
        fx->p[DelayEffect::dly_time_right].val.f = std::log2(endSeconds);
        patch.copy_globaldata(patch.globaldata);

        float changing = 0.f;

        for (int b = 0; b < 200; ++b)
        {
            runBlock(&changing);
        }

        return {held, changing};
    }

    /*
     * A crossfade between two taps holding correlated material cannot be made
     * transparent: the taps can land in antiphase and cancel at the midpoint whatever
     * the fade length. So the thing Clean mode controls is how OFTEN it pays that cost.
     *
     * Sweep the delay time continuously the way host automation would, through a 100%
     * wet, feedback free delay carrying a steady tone, and count the resulting dropouts.
     * Each retime shows up as a dip in the tone's level; a delay that chases the
     * automation block by block produces one every fade length, which is what gets heard
     * as the time being chewed rather than swept.
     */
    int retimeEventsDuringSweep(float toneHz, float fromSeconds, float toSeconds,
                                float sweepSeconds) const
    {
        auto surge = makeDelay();
        auto &patch = surge->storage.getPatch();
        auto *fx = &(patch.fx[fxslot_global2]);

        constexpr double twoPi{6.283185307179586};
        const double dPhase = twoPi * toneHz / sampleRate;
        double phase = 0.0;

        float dataL alignas(16)[BLOCK_SIZE], dataR alignas(16)[BLOCK_SIZE];
        std::vector<float> captured;

        auto runBlock = [&](bool keep) {
            for (int k = 0; k < BLOCK_SIZE; ++k)
            {
                dataL[k] = (float)std::sin(phase);
                dataR[k] = dataL[k];
                phase += dPhase;
            }

            surge->fx[fxslot_global2]->process(dataL, dataR);

            if (keep)
            {
                captured.insert(captured.end(), dataL, dataL + BLOCK_SIZE);
            }
        };

        // fill the line and settle
        for (int b = 0; b < (int)(2.5 * sampleRate / BLOCK_SIZE); ++b)
        {
            runBlock(false);
        }

        auto blocks = (int)(sweepSeconds * sampleRate / BLOCK_SIZE);
        auto from = std::log2(fromSeconds), to = std::log2(toSeconds);

        for (int b = 0; b < blocks; ++b)
        {
            auto v = (float)(from + (to - from) * b / (double)blocks);

            fx->p[DelayEffect::dly_time_left].val.f = v;
            fx->p[DelayEffect::dly_time_right].val.f = v;
            patch.copy_globaldata(patch.globaldata);

            runBlock(true);
        }

        // level per period of the tone, so the tone's own ripple is averaged away
        auto frame = (int)std::round(sampleRate / toneHz);
        std::vector<float> level;

        for (size_t i = 0; i + frame <= captured.size(); i += frame)
        {
            double s = 0;

            for (int k = 0; k < frame; ++k)
            {
                s += (double)captured[i + k] * captured[i + k];
            }

            level.push_back((float)std::sqrt(s / frame));
        }

        REQUIRE(level.size() > 50);

        std::vector<float> sorted(level);
        std::sort(sorted.begin(), sorted.end());
        auto steady = sorted[sorted.size() * 3 / 4];
        auto threshold = steady * 0.7f; // about 3 dB down

        int events = 0;
        bool inDip = false;

        for (auto v : level)
        {
            if (v < threshold && !inDip)
            {
                ++events;
                inDip = true;
            }
            else if (v >= threshold)
            {
                inDip = false;
            }
        }

        return events;
    }
};
} // namespace

TEST_CASE("Dual Delay Line Modes", "[fx]")
{
    SECTION("Tape mode glides to the new delay time")
    {
        DelayLineModeProbe probe;
        probe.lineModeLeft = DelayEffect::dly_line_tape;
        probe.lineModeRight = DelayEffect::dly_line_tape;

        auto idx = probe.impulseReturnIndex();

        INFO("Impulse returned at sample " << idx);
        // The tap is still gliding when the impulse catches up with it, so the impulse
        // comes back out well before the newly requested 0.2s
        REQUIRE(idx > 0.1 * DelayLineModeProbe::sampleRate);
        REQUIRE(idx < 0.2 * DelayLineModeProbe::sampleRate - 1000);
    }

    SECTION("Clean mode applies the new delay time immediately")
    {
        DelayLineModeProbe probe;
        probe.lineModeLeft = DelayEffect::dly_line_clean;
        probe.lineModeRight = DelayEffect::dly_line_clean;

        auto idx = probe.impulseReturnIndex();

        INFO("Impulse returned at sample " << idx);
        REQUIRE(idx == Approx(0.2 * DelayLineModeProbe::sampleRate).margin(64));
    }

    SECTION("Clean mode crossfades to the new delay time rather than splicing")
    {
        DelayLineModeProbe probe;
        probe.lineModeLeft = DelayEffect::dly_line_clean;
        probe.lineModeRight = DelayEffect::dly_line_clean;

        auto [held, changing] = probe.sineStepDiscontinuity();

        INFO("Largest output step was " << held << " while held and " << changing
                                        << " across the change");
        REQUIRE(changing < 3 * held);
    }

    SECTION("Clean mode keeps the mod LFO gliding so Rate and Depth still chorus")
    {
        DelayLineModeProbe unmodulated;
        unmodulated.lineModeLeft = DelayEffect::dly_line_clean;
        unmodulated.lineModeRight = DelayEffect::dly_line_clean;

        auto modulated = unmodulated;
        // a fast, deep vibrato, so the LFO never settles and every block asks the tap to
        // move by a meaningful amount
        modulated.modRate = 5.f;
        modulated.modDepth = 2.f;

        auto flat = unmodulated.sineStepDiscontinuity().first;
        auto chorused = modulated.sineStepDiscontinuity().first;

        INFO("Largest output step was " << flat << " unmodulated and " << chorused << " modulated");
        // pitch modulation raises the sine's own slope a little, but a tap that stepped
        // at block rate instead of gliding would be several times worse
        REQUIRE(chorused < 2 * flat);
    }

    SECTION("Clean mode retimes sparingly while the delay time is swept")
    {
        DelayLineModeProbe probe;
        probe.lineModeLeft = DelayEffect::dly_line_clean;
        probe.lineModeRight = DelayEffect::dly_line_clean;
        probe.startSeconds = 1.f;

        auto events = probe.retimeEventsDuringSweep(250.f, 1.f, 0.05f, 2.f);

        INFO("Dropped out " << events << " times across the sweep");
        // only the retimes whose taps land near antiphase actually dip, so this counts
        // audible dropouts rather than retimes. Spacing them by half a lap of the line
        // measures 10 here, which is also what Replika's Modern mode measures on
        // comparable material; shortening the fade tenfold, so that retimes come ten
        // times as often, measures 50
        REQUIRE(events < 20);
    }

    SECTION("A linked Right channel follows the Left channel's line mode")
    {
        DelayLineModeProbe probe;
        probe.lineModeLeft = DelayEffect::dly_line_clean;
        probe.lineModeRight = DelayEffect::dly_line_tape;
        probe.linkRight = true;
        probe.channel = 1;

        auto idx = probe.impulseReturnIndex();

        INFO("Impulse returned at sample " << idx);
        REQUIRE(idx == Approx(0.2 * DelayLineModeProbe::sampleRate).margin(64));
    }

    SECTION("An unlinked Right channel keeps its own line mode")
    {
        DelayLineModeProbe probe;
        probe.lineModeLeft = DelayEffect::dly_line_clean;
        probe.lineModeRight = DelayEffect::dly_line_tape;
        probe.linkRight = false;
        probe.channel = 1;

        auto idx = probe.impulseReturnIndex();

        INFO("Impulse returned at sample " << idx);
        REQUIRE(idx < 0.2 * DelayLineModeProbe::sampleRate - 1000);
    }
}
