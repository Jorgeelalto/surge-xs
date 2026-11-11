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

#if !defined(_M_ARM64EC)
#include "BBDEnsembleEffect.h"
#endif

#include "DelayEffect.h"
#include "Reverb1Effect.h"

using namespace std;

Effect *spawn_effect(int id, SurgeStorage *storage, FxStorage *fxdata, pdata *pd)
{
    // std::cout << "Spawn Effect " << _D(id) << std::endl;
    // Surge::Debug::stackTraceToStdout(7);
    switch (id)
    {
    case fxt_delay:
        return new DelayEffect(storage, fxdata, pd);
    case fxt_reverb:
        return new Reverb1Effect(storage, fxdata, pd);
    case fxt_ensemble:
#if defined(_M_ARM64EC)
        return nullptr;
#else
        return new BBDEnsembleEffect(storage, fxdata, pd);
#endif
    default:
        return 0;
    };
}

Effect::Effect(SurgeStorage *storage, FxStorage *fxdata, pdata *pd)
{
    // assert(storage);
    this->fxdata = fxdata;
    this->storage = storage;
    this->pd = pd;
    ringout = 10000000;
    if (pd)
    {
        for (int i = 0; i < n_fx_params; i++)
        {
            pd_float[i] = &pd[fxdata->p[i].id].f;
            pd_int[i] = &pd[fxdata->p[i].id].i;
        }
    }
}

bool Effect::process_ringout(float *dataL, float *dataR, bool indata_present)
{
    if (indata_present)
        ringout = 0;
    else
        ringout++;

    int d = get_ringout_decay();
    if ((d < 0) || (ringout < d) || (ringout == 0))
    {
        process(dataL, dataR);
        return true;
    }
    else
        process_only_control();
    return false;
}

void Effect::init_ctrltypes()
{
    for (int j = 0; j < n_fx_params; j++)
    {
        fxdata->p[j].modulateable = true;
        fxdata->p[j].set_type(ct_none);
    }
}
