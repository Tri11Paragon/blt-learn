#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <blt/math/math.h>
#include <imgui.h>

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


blt::generalized_matrix<State_t, 4, 4> states({
	{Frozen, Frozen, Frozen, Frozen},
	{Frozen, Hole, Frozen, Hole},
	{Frozen, Frozen, Frozen, Hole},
	{Hole, Frozen, Frozen, Goal}
});

auto V = blt::generalized_matrix<float, 4, 4>::make_empty();

template <typename... Args>
std::vector<ASResult_t> ms(Args&&... args) { return std::vector<ASResult_t>{std::forward<Args>(args)...}; }

std::array<std::array<std::vector<ASResult_t>, 4>, states.size()> policy = {
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
	},

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

int main() { blt::gfx::init(blt::gfx::window_data{"Learn Java", init, update, destroy}.setSyncInterval(1)); }
