/*
 *  <Short Description>
 *  Copyright (C) 2026  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <blt/std/types.h>
#include <blt/logging/logging.h>
#include <array>

#include "blt/iterator/zip.h"

constexpr blt::u32 MAX_CONNECTIONS = 8;

blt::u8 sadd8(const blt::u8 a, const blt::u8 b)
{
    blt::u8 c = a + b;
    if (c < a)
        c = 0xFF;
    return c;
}

struct synapse_t
{
    blt::u8 mask{};

    synapse_t()
    {
        reset();
    };

    [[nodiscard]] blt::u8 forward(const blt::u8 in) const
    {
        return in & mask;
    }

    void decay()
    {
        if (mask > 0)
            mask >>= 1;
    }

    void activate()
    {
        mask <<= 1;
        mask += 1;
    }

    void reset()
    {
        mask = 0xFF;
    }

    void clear()
    {
        mask = 0;
    }
};

struct neuron_t
{
    std::array<synapse_t, MAX_CONNECTIONS> input_synapses;
    synapse_t output;
    blt::u8 input_accumulator;
    blt::u8 delay;
    blt::u8 timer;
    blt::u8 activation_energy;

    neuron_t()
    {
        output.clear();
        activation_energy = delay = ((1 << 4) - 1);
        timer = 0;
        input_accumulator = 0;
    }

    void input(const blt::u32 id, const blt::u8 value)
    {
        input_accumulator = sadd8(input_accumulator, input_synapses[id].forward(value));
        input_synapses[id].activate();
    }

    std::optional<blt::u8> tick()
    {
        // decay inputs per tick.
        for (auto& i : input_synapses)
            i.decay();
        if (timer > 0)
            --timer;
        // output regains the ability to activate as time goes on.
        output.activate();
        if (input_accumulator > activation_energy && timer == 0)
        {
            auto ret = output.forward(input_accumulator);
            output.clear();
            input_accumulator = 0;
            return ret;
        }
        return {};
    }
};


int main()
{
    synapse_t synapse;

    for (int i = 0; i < 9; i++)
    {
        const auto in = static_cast<blt::u8>((1 << i) - 1);
        BLT_TRACE("{} | {:b} -> {:b}", i, in, synapse.forward(in));
        synapse.decay();
    }
}