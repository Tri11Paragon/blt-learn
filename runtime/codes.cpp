/*
*  Copyright (C) 2024  Brett Terpstra
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
#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <imgui.h>

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera camera;

struct code_t
{
    blt::u8 GF = 2;
    std::vector<blt::u8> data;

    explicit code_t(const std::vector<blt::u8>& data) : data(data)
    {
    }

    code_t(const std::initializer_list<blt::u8> data) : data(data)
    {
    }

    code_t(const std::initializer_list<blt::i32> d)
    {
        for (const auto v : d)
            data.push_back(static_cast<blt::u8>(v));
    }

    code_t() = default;

    void update(std::vector<blt::u8> new_data)
    {
        GF = 2;
        data = std::move(new_data);
        for (const auto c : data)
            GF = std::max(GF, static_cast<blt::u8>(c + 1));
    }

    auto begin()
    {
        return data.begin();
    }

    auto end()
    {
        return data.end();
    }

    [[nodiscard]] auto begin() const
    {
        return data.begin();
    }

    [[nodiscard]] auto end() const
    {
        return data.end();
    }

    [[nodiscard]] auto size() const
    {
        return data.size();
    }

    void resize(const size_t size)
    {
        data.resize(size);
    }

    [[nodiscard]] size_t distance_to(const code_t& o) const
    {
        size_t dist = 0;
        for (const auto [a, b] : blt::in_pairs(*this, o))
            dist += static_cast<size_t>(a != b);
        return dist;
    }

    static size_t distance(const code_t& a, const code_t& b)
    {
        return a.distance_to(b);
    }

    friend code_t operator+(const code_t& a, const code_t& b)
    {
        code_t r;
        r.resize(a.size());
        for (const auto [o, u, v] : blt::zip(r, a, b))
            o = (u + v) % a.GF;
        return r;
    }

    friend size_t operator*(const code_t& a, const code_t& b)
    {
        size_t dot = 0;
        for (const auto [u, v] : blt::in_pairs(a, b))
            dot += u * v;
        return dot % a.GF;
    }

    [[nodiscard]] size_t weight() const
    {
        size_t wt = 0;
        for (const auto c : *this)
            wt += static_cast<size_t>(c > 0);
        return wt;
    }

    explicit operator std::string() const
    {
        std::string ret;

        for (const auto c : *this)
        {
            ret += std::to_string(c);
            ret += ' ';
        }
        ret.pop_back();

        return ret;
    }

    [[nodiscard]] std::string to_string() const
    {
        return static_cast<std::string>(*this);
    }
};

std::vector<size_t> weight_distribution(const std::vector<code_t>& matrix)
{
    std::vector<size_t> dist;
    dist.resize(matrix.size());
    for (const auto [o, a] : blt::in_pairs(dist, matrix))
        o = a.weight();
    return dist;
}

std::vector<size_t> distance_distribution(const std::vector<code_t>& matrix)
{
    std::vector<size_t> dist;
    dist.resize(matrix.size());
    for (const auto paired : blt::in_pairs(dist, matrix))
    {
        auto& o = std::get<0>(paired);
        const auto& a = std::get<1>(paired);
        auto map = blt::iterate(matrix).map([&a](const auto& ele)
        {

            return a.distance_to(ele);
        });
        std::vector<size_t> distances;
        for (auto v : map)
            distances.push_back(v);
        o = *std::min_element(distances.begin(), distances.end());
    }
    return dist;
}

size_t min_weight(const std::vector<code_t>& matrix)
{
    auto dist = weight_distribution(matrix);
    return *std::min_element(dist.begin(), dist.end());
}

size_t inner_product(const code_t& a, const code_t& b)
{
    return a * b;
}

std::vector<code_t> generate_codewords(const std::vector<code_t>& generator_matrix)
{
    std::vector<code_t> codewords;

    for (size_t i = 0; i < generator_matrix.size(); i++)
    {
        for (size_t j = i; j < generator_matrix.size(); j++)
        {
            if (i == j)
            {
                codewords.push_back(generator_matrix[i]);
                continue;
            }
            codewords.push_back(generator_matrix[i] + generator_matrix[j]);
        }
    }

    return codewords;
}

void init(const blt::gfx::window_data&)
{
    using namespace blt::gfx;


    global_matrices.create_internals();
    resources.load_resources();
    renderer_2d.create();
}

void update(const blt::gfx::window_data& data)
{
    global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

    camera.update();
    camera.update_view(global_matrices);
    global_matrices.update();

    renderer_2d.render(data.width, data.height);
}

void destroy(const blt::gfx::window_data&)
{
    global_matrices.cleanup();
    resources.cleanup();
    renderer_2d.cleanup();
    blt::gfx::cleanup();
}

int main()
{
    const auto generator_matrix = std::vector<code_t>{
        {1, 0, 0, 0, 1, 1, 1},
        {0, 1, 0, 0, 1, 0, 0},
        {0, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 1, 0, 1, 1}
    };
    const auto codewords = generate_codewords(generator_matrix);
    for (const auto& [i, c] : blt::enumerate(codewords))
        BLT_TRACE("Code {}: {}", i, c.to_string());

    auto weights = weight_distribution(codewords);
    auto min_weight = *std::min_element(weights.begin(), weights.end());

    auto distances = distance_distribution(codewords);
    auto min_distance = *std::min_element(distances.begin(), distances.end());

    BLT_TRACE("{} vs {}", min_weight, min_distance);


    // blt::gfx::init(blt::gfx::window_data{"Codes for me? Codes for you", init, update, destroy}.setSyncInterval(1));
}
