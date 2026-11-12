#pragma once

#include "SurgeStorage.h"

#include <array>

class FixedFxSceneDelay
{
  public:
    void reset()
    {
        for (auto &scene : samples)
            for (auto &channel : scene)
                channel.fill(0.f);
        position = 0;
    }

    void process(const float input[2][2], float output[2][2])
    {
        for (int scene = 0; scene < 2; ++scene)
            for (int channel = 0; channel < 2; ++channel)
            {
                output[scene][channel] = samples[scene][channel][position];
                samples[scene][channel][position] = input[scene][channel];
            }
        position = (position + 1) % fixedFxLatencySamples;
    }

  private:
    std::array<std::array<std::array<float, fixedFxLatencySamples>, 2>, 2> samples{};
    int position{0};
};
