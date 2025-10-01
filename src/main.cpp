#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <blt/math/math.h>
#include <blt/std/random.h>
#include <imgui.h>

#include <utility>
#include <blt/std/system.h>
#include <blt/std/time.h>

blt::gfx::matrix_state_manager   global_matrices;
blt::gfx::resource_manager       resources;
blt::gfx::batch_renderer_2d      renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera_2d camera;

using blt::u32;


enum State_t : u32
{
	Start,
	Frozen,
	Hole,
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
	u32   next_state;
	bool  done;
};


using policy_action_t = std::array<float, 4>;
using policy_t        = std::array<policy_action_t, 16>;


struct map_t
{
	explicit map_t(std::vector<std::array<std::vector<ASResult_t>, 4>> data) : data{std::move(data)} {}

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

	static constexpr blt::generalized_matrix<State_t, 4, 4> states{
		std::array{
			Start,
			Frozen,
			Frozen,
			Frozen,

			Frozen,
			Hole,
			Frozen,
			Hole,

			Frozen,
			Frozen,
			Frozen,
			Hole,

			Hole,
			Frozen,
			Frozen,
			Goal
		}};

	std::vector<std::array<std::vector<ASResult_t>, 4>> data;
	float                                               beginX = 50;
	float                                               beginY = 50;
	float                                               padX   = 10;
	float                                               padY   = 10;
	float                                               width  = 50;
	float                                               height = 50;
};


map_t map{
	{
		// State 0:
		// State 0:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 0, false},
				ASResult_t{0.3333333333333333, 0, 0, false},
				ASResult_t{0.33333333333333337, 0, 4, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 0, false},
				ASResult_t{0.3333333333333333, 0, 4, false},
				ASResult_t{0.33333333333333337, 0, 1, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 4, false},
				ASResult_t{0.3333333333333333, 0, 1, false},
				ASResult_t{0.33333333333333337, 0, 0, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 1, false},
				ASResult_t{0.3333333333333333, 0, 0, false},
				ASResult_t{0.33333333333333337, 0, 0, false},},
		},

		// State 1:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 1, false},
				ASResult_t{0.3333333333333333, 0, 0, false},
				ASResult_t{0.33333333333333337, 0, 5, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 0, false},
				ASResult_t{0.3333333333333333, 0, 5, true},
				ASResult_t{0.33333333333333337, 0, 2, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 5, true},
				ASResult_t{0.3333333333333333, 0, 2, false},
				ASResult_t{0.33333333333333337, 0, 1, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 2, false},
				ASResult_t{0.3333333333333333, 0, 1, false},
				ASResult_t{0.33333333333333337, 0, 0, false},},
		},

		// State 2:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 2, false},
				ASResult_t{0.3333333333333333, 0, 1, false},
				ASResult_t{0.33333333333333337, 0, 6, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 1, false},
				ASResult_t{0.3333333333333333, 0, 6, false},
				ASResult_t{0.33333333333333337, 0, 3, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 6, false},
				ASResult_t{0.3333333333333333, 0, 3, false},
				ASResult_t{0.33333333333333337, 0, 2, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 3, false},
				ASResult_t{0.3333333333333333, 0, 2, false},
				ASResult_t{0.33333333333333337, 0, 1, false},},
		},

		// State 3:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 3, false},
				ASResult_t{0.3333333333333333, 0, 2, false},
				ASResult_t{0.33333333333333337, 0, 7, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 2, false},
				ASResult_t{0.3333333333333333, 0, 7, true},
				ASResult_t{0.33333333333333337, 0, 3, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 7, true},
				ASResult_t{0.3333333333333333, 0, 3, false},
				ASResult_t{0.33333333333333337, 0, 3, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 3, false},
				ASResult_t{0.3333333333333333, 0, 3, false},
				ASResult_t{0.33333333333333337, 0, 2, false},},
		},

		// State 4:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 0, false},
				ASResult_t{0.3333333333333333, 0, 4, false},
				ASResult_t{0.33333333333333337, 0, 8, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 4, false},
				ASResult_t{0.3333333333333333, 0, 8, false},
				ASResult_t{0.33333333333333337, 0, 5, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 8, false},
				ASResult_t{0.3333333333333333, 0, 5, true},
				ASResult_t{0.33333333333333337, 0, 0, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 5, true},
				ASResult_t{0.3333333333333333, 0, 0, false},
				ASResult_t{0.33333333333333337, 0, 4, false},},
		},

		// State 5:
		std::array{
			std::vector{ASResult_t{1.0, 0, 5, true},},
			std::vector{ASResult_t{1.0, 0, 5, true},},
			std::vector{ASResult_t{1.0, 0, 5, true},},
			std::vector{ASResult_t{1.0, 0, 5, true},},
		},

		// State 6:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 2, false},
				ASResult_t{0.3333333333333333, 0, 5, true},
				ASResult_t{0.33333333333333337, 0, 10, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 5, true},
				ASResult_t{0.3333333333333333, 0, 10, false},
				ASResult_t{0.33333333333333337, 0, 7, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 10, false},
				ASResult_t{0.3333333333333333, 0, 7, true},
				ASResult_t{0.33333333333333337, 0, 2, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 7, true},
				ASResult_t{0.3333333333333333, 0, 2, false},
				ASResult_t{0.33333333333333337, 0, 5, true},},
		},

		// State 7:
		std::array{
			std::vector{ASResult_t{1.0, 0, 7, true},},
			std::vector{ASResult_t{1.0, 0, 7, true},},
			std::vector{ASResult_t{1.0, 0, 7, true},},
			std::vector{ASResult_t{1.0, 0, 7, true},},
		},

		// State 8:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 4, false},
				ASResult_t{0.3333333333333333, 0, 8, false},
				ASResult_t{0.33333333333333337, 0, 12, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 8, false},
				ASResult_t{0.3333333333333333, 0, 12, true},
				ASResult_t{0.33333333333333337, 0, 9, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 12, true},
				ASResult_t{0.3333333333333333, 0, 9, false},
				ASResult_t{0.33333333333333337, 0, 4, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 9, false},
				ASResult_t{0.3333333333333333, 0, 4, false},
				ASResult_t{0.33333333333333337, 0, 8, false},},
		},

		// State 9:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 5, true},
				ASResult_t{0.3333333333333333, 0, 8, false},
				ASResult_t{0.33333333333333337, 0, 13, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 8, false},
				ASResult_t{0.3333333333333333, 0, 13, false},
				ASResult_t{0.33333333333333337, 0, 10, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 13, false},
				ASResult_t{0.3333333333333333, 0, 10, false},
				ASResult_t{0.33333333333333337, 0, 5, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 10, false},
				ASResult_t{0.3333333333333333, 0, 5, true},
				ASResult_t{0.33333333333333337, 0, 8, false},},
		},

		// State 10:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 6, false},
				ASResult_t{0.3333333333333333, 0, 9, false},
				ASResult_t{0.33333333333333337, 0, 14, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 9, false},
				ASResult_t{0.3333333333333333, 0, 14, false},
				ASResult_t{0.33333333333333337, 0, 11, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 14, false},
				ASResult_t{0.3333333333333333, 0, 11, true},
				ASResult_t{0.33333333333333337, 0, 6, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 11, true},
				ASResult_t{0.3333333333333333, 0, 6, false},
				ASResult_t{0.33333333333333337, 0, 9, false},},
		},

		// State 11:
		std::array{
			std::vector{ASResult_t{1.0, 0, 11, true},},
			std::vector{ASResult_t{1.0, 0, 11, true},},
			std::vector{ASResult_t{1.0, 0, 11, true},},
			std::vector{ASResult_t{1.0, 0, 11, true},},
		},

		// State 12:
		std::array{
			std::vector{ASResult_t{1.0, 0, 12, true},},
			std::vector{ASResult_t{1.0, 0, 12, true},},
			std::vector{ASResult_t{1.0, 0, 12, true},},
			std::vector{ASResult_t{1.0, 0, 12, true},},
		},

		// State 13:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 9, false},
				ASResult_t{0.3333333333333333, 0, 12, true},
				ASResult_t{0.33333333333333337, 0, 13, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 12, true},
				ASResult_t{0.3333333333333333, 0, 13, false},
				ASResult_t{0.33333333333333337, 0, 14, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 13, false},
				ASResult_t{0.3333333333333333, 0, 14, false},
				ASResult_t{0.33333333333333337, 0, 9, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 14, false},
				ASResult_t{0.3333333333333333, 0, 9, false},
				ASResult_t{0.33333333333333337, 0, 12, true},},
		},

		// State 14:
		std::array{
			std::vector{
				ASResult_t{0.33333333333333337, 0, 10, false},
				ASResult_t{0.3333333333333333, 0, 13, false},
				ASResult_t{0.33333333333333337, 0, 14, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 13, false},
				ASResult_t{0.3333333333333333, 0, 14, false},
				ASResult_t{0.33333333333333337, 1, 15, true},},
			std::vector{
				ASResult_t{0.33333333333333337, 0, 14, false},
				ASResult_t{0.3333333333333333, 1, 15, true},
				ASResult_t{0.33333333333333337, 0, 10, false},},
			std::vector{
				ASResult_t{0.33333333333333337, 1, 15, true},
				ASResult_t{0.3333333333333333, 0, 10, false},
				ASResult_t{0.33333333333333337, 0, 13, false},},
		},

		// State 15:
		std::array{
			std::vector{ASResult_t{1.0, 0, 15, true},},
			std::vector{ASResult_t{1.0, 0, 15, true},},
			std::vector{ASResult_t{1.0, 0, 15, true},},
			std::vector{ASResult_t{1.0, 0, 15, true},},
		}
	}
};

policy_t default_policy = {
	// state 0
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 1
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 2
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 3
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 4
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 5
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 6
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 7
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 8
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 9
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 10
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 11
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 12
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 13
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 14
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
	// state 16
	std::array{0.25f, 0.25f, 0.25f, 0.25f},
};

std::array<float, map_t::states.size()> evaluate_policy(const map_t&                         lmap,
														const policy_t&                      lpolicy,
														std::array<float, policy_t{}.size()> V,
														const float                          gamma,
														const u32                            I = 100,
														const float                          E = 0.00001f)
{
	std::array<float, policy_t{}.size()> Vp = V;

	for (u32 i = 0; i < I; i++)
	{
		float delta = 0;
		V           = Vp;
		for (u32 x = 0; x < V.size(); x++)
		{
			auto& ws = lmap.data[x];
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

policy_t greedy_policy(const map_t&                         lmap,
					   const policy_t&                      lpolicy,
					   std::array<float, policy_t{}.size()> V,
					   const float                          gamma)
{
	policy_t new_policy = lpolicy;

	for (u32 x = 0; x < V.size(); x++)
	{
		auto& policy_action = new_policy[x];
		auto& ms            = lmap.data[x];

		u32   action = 0;
		float value  = std::numeric_limits<float>::min();
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
				value  = total;
				action = a;
			}
		}

		for (auto& v : policy_action)
			v = 0;
		policy_action[action] = 1;
	}

	return new_policy;
}

void print(const std::array<float, policy_t{}.size()>& v)
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

std::array<float, policy_t{}.size()> V{};
policy_t                             current_policy = default_policy;
float                                Gamma          = 0.999;


struct agent_t
{
	bool update()
	{
		const auto next_state = current_policy[current_state];
		for (u32 i = 0; i < next_state.size(); i++)
		{
			// TODO: this doesn't guarantee that we will take an action
			// not to mention it is likely imbalanced
			if (random.choice(next_state[i]))
			{
				const auto vals = map.data[current_state][i];
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
	u32                   current_state = 0;
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
			auto       color = blt::make_color(1, 0, 0);
			const auto val   = map_t::states.m(i, j);

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
	bool        run_agent  = false;
	if (ImGui::Begin("Controls"))
	{
		if (agent_dead)
		{
			if (ImGui::Button("Respawn Agent"))
			{
				agent_dead = false;
				my_agent   = {};
			}
		} else
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

	if (run_agent) { agent_dead |= my_agent.update(); }

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

int main() { blt::gfx::init(blt::gfx::window_data{"Learn Java", init, update, destroy}.setSyncInterval(1)); }
