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
#include <set>
#include <thread>
#include <blt/profiling/profiler_v2.h>

#include "blt/std/random.h"

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

    [[nodiscard]] code_t noisy(const double error_chance = 0.1) const
    {
        thread_local blt::random::random_t random{std::random_device{}()};
        code_t code = *this;
        for (auto& bit : code)
        {
            if (random.choice(error_chance))
                bit = static_cast<blt::u8>(random.get_u32(0, GF));
        }
        return code;
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

    friend bool operator==(const code_t& a, const code_t& b)
    {
        return blt::in_pairs(a, b).map([](const auto& tuple)
        {
            return std::get<0>(tuple) == std::get<1>(tuple);
        }).all();
    }

    friend bool operator!=(const code_t& a, const code_t& b)
    {
        return !(a == b);
    }

    code_t operator++() const
    {
        code_t copy = *this;
        copy.data[copy.size() - 1]++;

        std::optional<blt::u8> carry = {};
        for (size_t i = 0; i < copy.size(); i++)
        {
            const size_t index = (copy.size() - 1) - i;
            auto& value = copy.data[index];
            if (carry)
            {
                value += *carry;
                carry = {};
            }
            const auto rem = value % copy.GF;
            if (rem == 0 && value != 0)
                carry = 1;
            value = static_cast<blt::u8>(rem);
        }

        return copy;
    }

    [[nodiscard]] size_t weight() const
    {
        size_t wt = 0;
        for (const auto c : *this)
            wt += static_cast<size_t>(c > 0);
        return wt;
    }

    [[nodiscard]] size_t dot(const code_t& other) const
    {
        return blt::in_pairs(*this, other).map([](const auto& tuple) -> size_t
        {
            const auto [a, b] = tuple;
            return a * b;
        }).sum() % other.GF;
    }

    explicit operator std::string() const
    {
        std::string ret{};
        if (data.empty())
            return ret;

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

struct code_hash_t
{
    size_t operator()(const code_t& code) const
    {
        size_t collector = 0;
        for (auto [i, v] : blt::enumerate(code))
            collector += i * v;
        return collector;
    }
};

struct code_set_t
{
    blt::hashset_t<code_t, code_hash_t> contained_codes;
    std::vector<code_t> codes;

    // returns true if the code was added.
    bool add_code(const code_t& code)
    {
        if (contained_codes.find(code) != contained_codes.end())
            return false;
        contained_codes.insert(code);
        codes.push_back(code);
        return true;
    }

    [[nodiscard]] code_t multiply(const code_t& code) const
    {
        BLT_ASSERT_MSG(codes.size() == code.size(),
                       "Expected input code size to be equal to the number of codes in the set. (k)");
        const auto g = code.size();
        const auto n = codes.begin()->data.size();

        code_t w;
        w.data.resize(n);

        // output = 1 x n

        // in A = 1 x k
        // in B = k x n

        for (blt::u32 j = 0; j < n; j++)
        {
            for (blt::u32 k = 0; k < g; k++)
                w.data[j] = w.data[j] + code.data[k] * codes[k].data[j];
        }
        for (blt::u32 j = 0; j < n; j++)
            w.data[j] = w.data[j] % code.GF;

        return w;
    }

    [[nodiscard]] auto gf() const
    {
        BLT_ASSERT(!codes.empty());
        return codes.begin()->GF;
    }

    [[nodiscard]] auto k() const
    {
        return codes.size();
    }

    [[nodiscard]] auto n() const
    {
        BLT_ASSERT(!codes.empty());
        return codes.begin()->size();
    }

    auto operator[](const size_t index) -> decltype(auto)
    {
        return codes[index];
    }

    auto operator[](const size_t index) const -> decltype(auto)
    {
        return codes[index];
    }

    [[nodiscard]] auto size() const
    {
        return k();
    }

    friend bool operator==(const code_set_t& a, const code_set_t& b)
    {
        if (a.size() != b.size())
            return false;
        for (const auto& code : a.codes)
        {
            if (b.contained_codes.find(code) == b.contained_codes.end())
                return false;
        }
        return true;
    }

    friend bool operator!=(const code_set_t& a, const code_set_t& b)
    {
        return !(a == b);
    }

    auto begin()
    {
        return codes.begin();
    }

    [[nodiscard]] auto begin() const
    {
        return codes.begin();
    }

    auto end()
    {
        return codes.end();
    }

    [[nodiscard]] auto end() const
    {
        return codes.end();
    }

    [[nodiscard]] bool is_orthogonal(const code_t& code) const
    {
        return blt::iterate(*this).map([](const code_t& c, const code_t& other)
        {
            return other.dot(c) == 0;
        }, code).all();
    }

    [[nodiscard]] std::vector<size_t> weight_distribution() const
    {
        std::vector<size_t> dist;
        dist.resize(size());
        for (const auto [o, a] : blt::in_pairs(dist, *this))
            o = a.weight();
        return dist;
    }

    [[nodiscard]] std::vector<size_t> distance_distribution() const
    {
        std::vector<size_t> dist;
        dist.resize(size());
        for (const auto paired : blt::in_pairs(dist, *this))
        {
            auto& o = std::get<0>(paired);
            const auto& a = std::get<1>(paired);
            auto distances = blt::iterate(*this).map([](const auto& ele, const auto& a)
            {
                return a.distance_to(ele);
            }, a).filter(blt::iterator::greater, 0).collect();
            if (distances.empty())
                continue;
            o = *std::min_element(distances.begin(), distances.end());
        }
        return dist;
    }

    [[nodiscard]] code_set_t generate_codewords() const
    {
        code_set_t codewords;

        const auto k = this->k();

        code_t code{};
        code.data.resize(k);
        for (size_t ik = 0; ik < static_cast<size_t>(std::pow(this->gf(), k)); ik++)
        {
            codewords.add_code(this->multiply(code));
            code = ++code;
        }

        return codewords;
    }

    [[nodiscard]] code_set_t dual() const
    {
        code_set_t dual;

        const auto n = this->n();

        code_t code{};
        code.data.resize(n);
        for (size_t ik = 0; ik < static_cast<size_t>(std::pow(this->gf(), n)); ik++)
        {
            if (is_orthogonal(code))
                dual.add_code(code);
            code = ++code;
        }

        return dual;
    }
};

size_t min_weight(const code_set_t& matrix)
{
    auto dist = matrix.weight_distribution();
    return *std::min_element(dist.begin(), dist.end());
}

size_t inner_product(const code_t& a, const code_t& b)
{
    return a * b;
}

struct less_lex
{
    bool operator()(const code_t& a, const code_t& b) const
    {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
    }
};

using set_t = std::set<code_t, less_lex>;

set_t build_base_set(const blt::u32 q, const blt::u32 n)
{
    set_t ret;
    code_t code{};
    code.data.resize(n);
    for (size_t ik = 0; ik < static_cast<size_t>(std::pow(q, n)); ik++)
    {
        ret.insert(code);
        code = ++code;
    }
    return ret;
}

struct code_not_found_t
{
};

struct backtrack
{
    blt::u32 n, M, d, q;

    std::vector<code_set_t> found_codes{};

    std::vector<code_t> code;
    std::vector<set_t> candidates;

    backtrack(const blt::u32 n, const blt::u32 M, const blt::u32 d, const blt::u32 q) : n(n), M(M), d(d), q(q)
    {
        candidates.push_back(build_base_set(q, n));
    }

    void basic(const blt::u32 level = 0)
    {
        // if (candidates[level].empty())
        //     throw code_not_found_t{};
        if (level + 1 >= candidates.size())
            candidates.emplace_back();
        if (level >= code.size())
            code.emplace_back();
        auto v = candidates[level].begin();
        for (; v != candidates[level].end(); ++v)
        {
            code[level] = *v;
            candidates[level + 1] = {};
            for (auto w = std::next(v); w != candidates[level].end(); ++w)
            {
                if (v->distance_to(*w) >= d)
                    candidates[level + 1].insert(*w);
            }
            if (level < M)
                basic(level + 1);
            else
            {
                code_set_t found_code{};
                for (const auto& c : code)
                    BLT_ASSERT(found_code.add_code(c));
                found_codes.push_back(found_code);
            }
        }
    }
};

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

void generator()
{
    auto generator_matrix = code_set_t{};
    generator_matrix.add_code({1, 0, 0, 0, 1, 1, 1});
    generator_matrix.add_code({0, 1, 0, 0, 1, 0, 0});
    generator_matrix.add_code({0, 0, 1, 0, 1, 0, 1});
    generator_matrix.add_code({0, 0, 0, 1, 0, 1, 1});

    const auto codewords = generator_matrix.generate_codewords();
    for (const auto& [i, c] : blt::enumerate(codewords))
        BLT_TRACE("Code {}: {}", i, c.to_string());

    const auto codewords2 = codewords.generate_codewords();
    for (const auto& [i, c] : blt::enumerate(codewords2))
        BLT_TRACE("Code2 {}: {}", i, c.to_string());

    BLT_TRACE("ARE THEY EQUAL? {}", codewords == codewords2);

    const auto dual = generator_matrix.dual();
    for (const auto& [i, c] : blt::enumerate(dual))
        BLT_TRACE("Dual {}: {}", i, c.to_string());

    const auto weights = codewords.weight_distribution();
    std::optional<size_t> min_weight;
    for (const auto& w : weights)
    {
        if (w != 0)
            min_weight = std::min(w, min_weight.value_or(std::numeric_limits<size_t>::max()));
    }

    auto distances = codewords.distance_distribution();
    auto min_distance = *std::min_element(distances.begin(), distances.end());

    BLT_TRACE("{} vs {}", min_weight.value_or(0), min_distance);
}

int main()
{
    generator();
    BLT_START_INTERVAL("Codes", "Backtracker");
    backtrack tracker{4, 4, 2, 2};
    try
    {
        tracker.basic();
    }
    catch (code_not_found_t)
    {
        BLT_INFO("Unable to find code ({}, {}, {})_{}", tracker.n, tracker.M, tracker.d, tracker.q);
        BLT_TRACE("Managed to find {} codewords before failing!", tracker.code.size());
    }
    BLT_END_INTERVAL("Codes", "Backtracker");
    BLT_START_INTERVAL("Codes", "Code Size Analysis");

    BLT_INFO("Found {} codes", tracker.found_codes.size());
    auto iter = blt::iterate(tracker.found_codes).map([](const auto& a)
    {
        return a.size();
    }).collect();
    if (!iter.empty())
    {
        auto min = *std::min_element(iter.begin(), iter.end());
        auto max = *std::max_element(iter.begin(), iter.end());
        BLT_INFO("Lower Bound: {} | Upper Bound: {}", min, max);
    }
    BLT_END_INTERVAL("Codes", "Code Size Analysis");
    BLT_PRINT_PROFILE("Codes");
    // for (auto& codewords : tracker.found_codes)
    // {
    //     BLT_TRACE("Found a ({}, {}, {})_{} with {} codewords!", tracker.n, tracker.M, tracker.d, tracker.q,
    //           codewords.size());
    // }
    // for (const auto& code : tracker.code)
    // {
    //     // std::cout << code.to_string() << std::endl;
    //     BLT_TRACE("{}", code.to_string());
    // }
    // BLT_TRACE("Found a ({}, {}, {})_{} with {} codewords!", tracker.n, tracker.M, tracker.d, tracker.q,
    //           tracker.code.size());


    // blt::gfx::init(blt::gfx::window_data{"Codes for me? Codes for you", init, update, destroy}.setSyncInterval(1));
}
