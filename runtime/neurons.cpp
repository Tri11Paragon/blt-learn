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

#include "blt/iterator/enumerate.h"
#include "blt/iterator/zip.h"
#include "blt/std/hashmap.h"

constexpr blt::u32 MAX_CONNECTIONS = 8;

enum class neuron_type_t
{
    HIDDEN, INPUT, OUTPUT
};

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
            BLT_TRACE("{} ? {} | {}", activation_energy, input_accumulator, output.mask);
            auto ret = output.forward(input_accumulator);
            output.clear();
            input_accumulator = 0;
            timer = delay;
            return ret;
        }
        return {};
    }
};


struct neuron_structure_t
{
    blt::hashmap_t<std::string, blt::size_t> neuron_ids;
    std::vector<neuron_t> neurons;
    blt::hashmap_t<blt::size_t, blt::size_t> connections;
    blt::hashmap_t<blt::size_t, blt::u32> connection_locations;

    std::vector<blt::size_t> output_neurons;
    std::vector<blt::size_t> input_neurons;

    blt::size_t add_neuron(const std::string& name, const neuron_type_t type = neuron_type_t::HIDDEN)
    {
        const auto id = neurons.size();
        neurons.emplace_back();
        neuron_ids[name] = id;

        switch (type)
        {
        case neuron_type_t::HIDDEN:
            break;
        case neuron_type_t::INPUT:
            input_neurons.push_back(id);
            break;
        case neuron_type_t::OUTPUT:
            output_neurons.push_back(id);
            break;
        }

        return id;
    }

    void connect(const std::string& from, const std::string& to)
    {
        connections[neuron_ids[from]] = neuron_ids[to];

    }

    void connect(const blt::size_t from, const blt::size_t to)
    {
        connections[from] = to;
    }

    void tick()
    {
        thread_local std::vector<blt::u8> outputs;
        outputs.clear();

        for (auto& n : neurons)
            outputs.push_back(n.tick().value_or(0));

        for (const auto& [i, out] : blt::enumerate(outputs))
        {
            if (out > 0)
                neurons[i].input(connections[i], out);
        }
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

    neuron_t n;
    for (int i = 0; i < 10; i++)
    {
        n.input(0, 0xFF);
        n.input(1, 0xF);
        const auto result = n.tick();
        if (result)
        {
            BLT_INFO("Neuron Fired {}", *result);
        }
    }
}