#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <blt/math/math.h>
#include <blt/std/random.h>
#include <imgui.h>
#include <stack>
#include <blt/math/v2/algebra.h>

#include <utility>
#include <blt/std/system.h>
#include <blt/std/time.h>

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera_2d camera;

using blt::u32;


enum State_t : u32
{
    Start,
    Frozen,
    Hole,
    Land,
    Goal
};

enum Action_t : u32
{
    Left,
    Down,
    Right,
    Up
};


struct ASResult_t
{
    float prob;
    float reward;
    u32 next_state;
    bool done;
};


using policy_action_t = std::array<float, 4>;
using policy_t = std::vector<policy_action_t>;
using V_t = std::vector<float>;

struct StateData_t
{
    std::array<std::vector<ASResult_t>, 4> action_results;
    State_t state = Start;

    StateData_t() = default;

    StateData_t(const State_t state, const std::array<std::vector<ASResult_t>, 4>& action_results)
        : action_results(action_results),
          state(state)
    {
    }
};

struct candidate_map_t
{
    std::vector<std::vector<State_t>> data;
    blt::hashmap_t<State_t, float> weights;
    blt::hashmap_t<State_t, blt::hashmap_t<State_t, blt::hashset_t<Action_t>>> compatibility;
    blt::random::random_t& random;
    u32 rows, columns;

    candidate_map_t(const u32 rows, const u32 columns, blt::hashmap_t<State_t, float> weights,
                    blt::random::random_t& random) : weights{
                                                         std::move(weights)
                                                     }, random{random},
                                                     rows{rows}, columns{columns}
    {
        data.resize(rows * columns);
        for (auto& row : data)
            row = {Start, Frozen, Hole, Land, Goal};
    }

    void generate_start()
    {
        const u32 startX = random.get_u32(0, rows);
        const u32 startY = random.get_u32(0, columns);
        possible_states(startX, startY) = {Start};
    }

    [[nodiscard]] const std::vector<State_t>& possible_states(const u32 r, const u32 c) const
    {
        return data[r * columns + c];
    }

    std::vector<State_t>& possible_states(const u32 r, const u32 c)
    {
        return data[r * columns + c];
    }

    [[nodiscard]] bool is_complete() const
    {
        for (const auto& row : data)
            if (row.size() != 1)
                return false;
        return true;
    }

    [[nodiscard]] std::vector<StateData_t> generate_states() const
    {
        std::vector<StateData_t> states;
        for (u32 r = 0; r < rows; r++)
        {
            for (u32 c = 0; c < columns; c++)
            {
                auto& ps = possible_states(r, c);
                BLT_ASSERT(ps.size() == 1);
                states[r * columns + c].state = ps.front();
            }
        }

        return states;
    }
};

struct map_t
{
    map_t(const u32 rows, const u32 columns, const blt::i32 min_distance = 1) : number_of_states{rows * columns},
        rows{rows}, columns{columns}
    {
        data.resize(number_of_states);
        const u32 startX = random.get_u32(0, rows);
        const u32 startY = random.get_u32(0, columns);
        get_state(startX, startY).state = Start;

        // u32 goalX = 0;
        // u32 goalY = 0;
        //
        // do
        // {
        //     goalX = random.get_u32(0, rows);
        //     goalY = random.get_u32(0, columns);
        // } while (std::abs(static_cast<blt::i32>(startX) - static_cast<blt::i32>(goalX)) <= min_distance &&
        //          std::abs(static_cast<blt::i32>(startY) - static_cast<blt::i32>(goalY)) <= min_distance);
        // get_state(goalX, goalY).state = Goal;

        std::stack<blt::vec2ui> visit;
        visit.push({startX, startY});

        while (!visit.empty())
        {
            auto top = visit.top();
            visit.pop();

            auto& state = get_state(top);
            if (state.state != UNGENERATED)
                continue;
            const auto up = state_up(top);
            const auto down = state_down(top);
            const auto left = state_left(top);
            const auto right = state_right(top);

            static blt::hashset_t<State_t> candidates;
            candidates.clear();

            if (up && up->state != UNGENERATED)
                candidates.insert(up->state);
            if (down && down->state != UNGENERATED)
                candidates.insert(down->state);
            if (left && left->state != UNGENERATED)
                candidates.insert(left->state);
            if (right && right->state != UNGENERATED)
                candidates.insert(right->state);

            if (candidates.contains(Frozen))
            {
            }
        }
    }

    explicit map_t(std::vector<StateData_t> data) : data{std::move(data)},
                                                    number_of_states{data.size()},
                                                    rows{
                                                        static_cast<u32>(std::log2(
                                                            data.size()))
                                                    },
                                                    columns{
                                                        static_cast<u32>(std::log2(
                                                            data.size()))
                                                    }
    {
    }

    State_t ensure_valid_state(State_t s)
    {
        switch (s)
        {
        case Start:
            while (true)
            {
                auto v = random.get_u32(0, generation_chances.size());
                if (generation_chances[static_cast<State_t>(v)] != 0 && random.choice(
                    generation_chances[static_cast<State_t>(v)]))
                    return static_cast<State_t>(v);
            }
        }
    }

    [[nodiscard]] blt::vec2 get_pos(const float i, const float j) const
    {
        return {beginX + width * i + padX * i, beginY + height * j + padY * j};
    }

    [[nodiscard]] blt::vec2 get_center_pos(const float i, const float j) const
    {
        return get_pos(i, j) + blt::vec2{width, height} / 2;
    }

    [[nodiscard]] blt::vec2 get_pos(const u32 s) const
    {
        return get_pos(static_cast<float>(s % 4), static_cast<float>(s / 4));
    }

    [[nodiscard]] blt::vec2 get_center_pos(const u32 s) const { return get_pos(s) + blt::vec2{width, height} / 2; }

    blt::vec2 get_size() { return {width, height}; }

    [[nodiscard]] size_t get_number_of_states() const
    {
        return number_of_states;
    }

    [[nodiscard]] u32 get_rows() const
    {
        return rows;
    }

    [[nodiscard]] u32 get_columns() const
    {
        return columns;
    }

    StateData_t& get_state(const u32 state)
    {
        return data[state];
    }

    [[nodiscard]] const StateData_t& get_state(const u32 state) const
    {
        return data[state];
    }

    StateData_t& get_state(const u32 r, const u32 c)
    {
        return data[r * columns + c];
    }

    [[nodiscard]] const StateData_t& get_state(const u32 r, const u32 c) const
    {
        return data[r * columns + c];
    }

    StateData_t& get_state(const blt::vec2ui& pos)
    {
        return data[pos.x() * columns + pos.y()];
    }

    [[nodiscard]] const StateData_t& get_state(const blt::vec2ui& pos) const
    {
        return data[pos.x() * columns + pos.y()];
    }

    [[nodiscard]] policy_t generate_default_policy() const
    {
        policy_t policy;
        policy.resize(number_of_states, std::array{0.25f, 0.25f, 0.25f, 0.25f});
        return policy;
    }

    [[nodiscard]] policy_t generate_random_policy()
    {
        policy_t policy;
        for (size_t i = 0; i < number_of_states; i++)
        {
            float remaining = 1.0f;
            const auto v1 = random.get_float(0, remaining);
            remaining -= v1;
            const auto v2 = random.get_float(0, remaining);
            remaining -= v2;
            const auto v3 = random.get_float(0, remaining);
            remaining -= v3;

            policy.emplace_back(std::array{v1, v2, v3, remaining});
        }
        return policy;
    }

    blt::random::random_t& get_random()
    {
        return random;
    }

private:
    blt::random::random_t random{std::random_device()()};
    std::vector<StateData_t> data;
    size_t number_of_states;
    u32 rows, columns;
    float beginX = 50;
    float beginY = 50;
    float padX = 10;
    float padY = 10;
    float width = 50;
    float height = 50;

    [[nodiscard]] std::optional<u32> state_left(const u32 r, const u32 c) const
    {
        const auto v = static_cast<blt::i32>(c) - 1;
        if (v < 0)
            return {};
        return r * columns + v;
    }

    [[nodiscard]] std::optional<u32> state_right(const u32 r, const u32 c) const
    {
        const auto v = c + 1;
        if (v >= columns)
            return {};
        return r * columns + v;
    }

    [[nodiscard]] std::optional<u32> state_up(const u32 r, const u32 c) const
    {
        const auto v = static_cast<blt::i32>(r) - 1;
        if (v < 0)
            return {};
        return v * columns + c;
    }

    [[nodiscard]] std::optional<u32> state_down(const u32 r, const u32 c) const
    {
        const auto v = r + 1;
        if (v >= rows)
            return {};
        return v * columns + c;
    }

    [[nodiscard]] std::optional<StateData_t> state_left(const blt::vec2ui pos) const
    {
        if (const auto val = state_left(pos.x(), pos.y()))
            return get_state(*val);
        return {};
    }

    [[nodiscard]] std::optional<StateData_t> state_right(const blt::vec2ui pos) const
    {
        if (const auto val = state_right(pos.x(), pos.y()))
            return get_state(*val);
        return {};
    }

    [[nodiscard]] std::optional<StateData_t> state_up(const blt::vec2ui pos) const
    {
        if (const auto val = state_up(pos.x(), pos.y()))
            return get_state(*val);
        return {};
    }

    [[nodiscard]] std::optional<StateData_t> state_down(const blt::vec2ui pos) const
    {
        if (const auto val = state_down(pos.x(), pos.y()))
            return get_state(*val);
        return {};
    }
};


map_t map{
    4, 4
};

V_t evaluate_policy(const map_t& lmap,
                    const policy_t& lpolicy,
                    V_t V,
                    const float gamma,
                    const u32 I = 100,
                    const float E = 0.00001f)
{
    auto Vp = V;

    for (u32 i = 0; i < I; i++)
    {
        float delta = 0;
        V = Vp;
        for (u32 x = 0; x < V.size(); x++)
        {
            auto& ws = lmap.get_state(x).action_results;
            auto& ps = lpolicy[x];

            float v = 0;

            for (const auto [p_a, w_a] : blt::in_pairs(ps, ws))
            {
                float l = 0;
                for (const auto& ns : w_a)
                    l += ns.prob * (ns.reward + (gamma * V[ns.next_state]));
                v += p_a * l;
            }

            delta = std::max(delta, std::abs(V[x] - v));
            Vp[x] = v;
        }

        if (delta < E)
        {
            BLT_TRACE("Ending after {} iterations with delta {}", i, delta);
            break;
        }
    }

    return Vp;
}

policy_t greedy_policy(const map_t& lmap,
                       const policy_t& lpolicy,
                       V_t V,
                       const float gamma)
{
    policy_t new_policy = lpolicy;

    for (u32 x = 0; x < V.size(); x++)
    {
        auto& policy_action = new_policy[x];
        auto& ms = lmap.get_state(x).action_results;

        u32 action = 0;
        float value = std::numeric_limits<float>::min();
        for (u32 a = 0; a < policy_action.size(); a++)
        {
            float r = 0;
            float l = 0;
            for (const auto& v : ms[a])
            {
                r += v.prob * v.reward;
                l += v.prob * V[v.next_state];
            }
            const float total = r + gamma * l;
            if (total > value)
            {
                value = total;
                action = a;
            }
        }

        for (auto& v : policy_action)
            v = 0;
        policy_action[action] = 1;
    }

    return new_policy;
}

void print(const V_t& v)
{
    std::cout << "V: [\n\t";
    for (const auto& [i, x] : blt::enumerate(v))
    {
        std::cout << x << " ";
        if (i % 4 == 3)
            std::cout << std::endl << "\t";
    }
    std::cout << "]" << std::endl;
}

V_t V{};
policy_t current_policy = map.generate_random_policy();
float Gamma = 0.999;


struct agent_t
{
    bool update(map_t& lmap)
    {
        const auto next_state = current_policy[current_state];
        for (u32 i = 0; i < next_state.size(); i++)
        {
            // TODO: this doesn't guarantee that we will take an action
            // not to mention it is likely imbalanced
            if (random.choice(next_state[i]))
            {
                const auto vals = lmap.get_state(current_state).action_results[i];
                for (const auto& v : vals)
                {
                    // which means we might just not move into next state.
                    // not sure if this is something you want!
                    if (random.choice(v.prob))
                    {
                        current_state = v.next_state;
                        if (v.done)
                            return true;
                    }
                }
            }
        }
        return false;
    }

    blt::random::random_t random{std::random_device()()};
    u32 current_state = 0;
};


agent_t my_agent;

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


    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            auto color = blt::make_color(1, 0, 0);
            const auto val = map.get_state(i, j).state;

            if (val == Goal)
                color = blt::make_color(0, 1, 0);
            else if (val == Hole)
                color = blt::make_color(0, 0, 1);
            else if (val == Start)
                color = blt::make_color(1, 0, 1);

            renderer_2d.drawRectangle({map.get_pos(i, j), map.get_size()}, color);
        }
    }

    static bool agent_dead = false;
    static bool continuous_run = false;
    static int time_between_runs = 50;
    static blt::u64 last_time_ran = 0;
    bool run_agent = false;
    if (ImGui::Begin("Controls"))
    {
        if (agent_dead)
        {
            if (ImGui::Button("Respawn Agent"))
            {
                agent_dead = false;
                my_agent = {};
            }
        }
        else
            run_agent |= ImGui::Button("Run Agent Step");
        ImGui::Checkbox("Continuous Run", &continuous_run);
        ImGui::SliderInt("Time Between Runs", &time_between_runs, 0, 1000);
        if (continuous_run)
        {
            if (agent_dead)
            {
                agent_dead = false;
                my_agent = {};
            }
            if (blt::system::getCurrentTimeMilliseconds() - last_time_ran > static_cast<blt::u64>(time_between_runs))
            {
                run_agent = true;
                last_time_ran = blt::system::getCurrentTimeMilliseconds();
            }
        }
        if (ImGui::Button("Evaluate & Update Policy"))
        {
            V = {};
            V = evaluate_policy(map, current_policy, V, Gamma);
            current_policy = greedy_policy(map, current_policy, V, Gamma);
            print(V);
        }
    }
    ImGui::End();

    if (run_agent) { agent_dead |= my_agent.update(map); }

    renderer_2d.drawPoint(blt::gfx::point2d_t{map.get_pos(my_agent.current_state), 25},
                          blt::make_color(.8, .8, .8),
                          1);

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
    const blt::matrix_t<blt::detail::static_matrix_t<float, 4, 4>, blt::detail::set_identity_t, blt::detail::print_t>
        mat1{};
    const blt::matrix_t<blt::detail::static_matrix_t<float, 4, 4>, blt::detail::set_identity_t, blt::detail::print_t>
        mat2{
            0, 0, 0, 100,
            0, 0, 0, 100,
            0, 0, 0, 100,
            0, 0, 0, 1
        };
    const auto ret = mat1.set_identity();
    ret.print(std::cout);
    BLT_TRACE("");
    mat2.print(std::cout);

    auto mul = ret * mat2;

    BLT_TRACE("");
    mul.print(std::cout);
    mul = mul.set_identity();
    mul.print(std::cout);

    BLT_TRACE("{}", __cplusplus);
    blt::gfx::init(blt::gfx::window_data{"Learn Java", init, update, destroy}.setSyncInterval(1));
}
