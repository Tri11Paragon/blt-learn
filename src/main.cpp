#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <blt/math/math.h>
#include <imgui.h>

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


blt::generalized_matrix<State_t, 4, 4> states({
    {Frozen, Frozen, Frozen, Frozen},
    {Frozen, Hole, Frozen, Hole},
    {Frozen, Frozen, Frozen, Hole},
    {Hole, Frozen, Frozen, Goal}
});

struct policy_action_t
{
    float prob;
    std::vector<ASResult_t> action;
};

using policy_t = std::array<std::array<policy_action_t, 4>, states.size()>;

policy_t policy = {
    // State 0:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 0, false},
                ASResult_t{0.3333333333333333, 0, 0, false},
                ASResult_t{0.33333333333333337, 0, 4, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 0, false},
                ASResult_t{0.3333333333333333, 0, 4, false},
                ASResult_t{0.33333333333333337, 0, 1, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 4, false},
                ASResult_t{0.3333333333333333, 0, 1, false},
                ASResult_t{0.33333333333333337, 0, 0, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 1, false},
                ASResult_t{0.3333333333333333, 0, 0, false},
                ASResult_t{0.33333333333333337, 0, 0, false},
            }
        },
    },

    // State 1:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 1, false},
                ASResult_t{0.3333333333333333, 0, 0, false},
                ASResult_t{0.33333333333333337, 0, 5, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 0, false},
                ASResult_t{0.3333333333333333, 0, 5, true},
                ASResult_t{0.33333333333333337, 0, 2, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 5, true},
                ASResult_t{0.3333333333333333, 0, 2, false},
                ASResult_t{0.33333333333333337, 0, 1, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 2, false},
                ASResult_t{0.3333333333333333, 0, 1, false},
                ASResult_t{0.33333333333333337, 0, 0, false},
            }
        },
    },

    // State 2:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 2, false},
                ASResult_t{0.3333333333333333, 0, 1, false},
                ASResult_t{0.33333333333333337, 0, 6, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 1, false},
                ASResult_t{0.3333333333333333, 0, 6, false},
                ASResult_t{0.33333333333333337, 0, 3, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 6, false},
                ASResult_t{0.3333333333333333, 0, 3, false},
                ASResult_t{0.33333333333333337, 0, 2, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 3, false},
                ASResult_t{0.3333333333333333, 0, 2, false},
                ASResult_t{0.33333333333333337, 0, 1, false},
            }
        },
    },

    // State 3:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 3, false},
                ASResult_t{0.3333333333333333, 0, 2, false},
                ASResult_t{0.33333333333333337, 0, 7, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 2, false},
                ASResult_t{0.3333333333333333, 0, 7, true},
                ASResult_t{0.33333333333333337, 0, 3, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 7, true},
                ASResult_t{0.3333333333333333, 0, 3, false},
                ASResult_t{0.33333333333333337, 0, 3, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 3, false},
                ASResult_t{0.3333333333333333, 0, 3, false},
                ASResult_t{0.33333333333333337, 0, 2, false},
            }
        },
    },

    // State 4:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 0, false},
                ASResult_t{0.3333333333333333, 0, 4, false},
                ASResult_t{0.33333333333333337, 0, 8, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 4, false},
                ASResult_t{0.3333333333333333, 0, 8, false},
                ASResult_t{0.33333333333333337, 0, 5, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 8, false},
                ASResult_t{0.3333333333333333, 0, 5, true},
                ASResult_t{0.33333333333333337, 0, 0, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 5, true},
                ASResult_t{0.3333333333333333, 0, 0, false},
                ASResult_t{0.33333333333333337, 0, 4, false},
            }
        },
    },

    // State 5:
    std::array{
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 5, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 5, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 5, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 5, true},}
        },
    },

    // State 6:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 2, false},
                ASResult_t{0.3333333333333333, 0, 5, true},
                ASResult_t{0.33333333333333337, 0, 10, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 5, true},
                ASResult_t{0.3333333333333333, 0, 10, false},
                ASResult_t{0.33333333333333337, 0, 7, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 10, false},
                ASResult_t{0.3333333333333333, 0, 7, true},
                ASResult_t{0.33333333333333337, 0, 2, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 7, true},
                ASResult_t{0.3333333333333333, 0, 2, false},
                ASResult_t{0.33333333333333337, 0, 5, true},
            }
        },
    },

    // State 7:
    std::array{
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 7, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 7, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 7, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 7, true},}
        }
    },

    // State 8:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 4, false},
                ASResult_t{0.3333333333333333, 0, 8, false},
                ASResult_t{0.33333333333333337, 0, 12, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 8, false},
                ASResult_t{0.3333333333333333, 0, 12, true},
                ASResult_t{0.33333333333333337, 0, 9, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 12, true},
                ASResult_t{0.3333333333333333, 0, 9, false},
                ASResult_t{0.33333333333333337, 0, 4, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 9, false},
                ASResult_t{0.3333333333333333, 0, 4, false},
                ASResult_t{0.33333333333333337, 0, 8, false},
            }
        },
    },

    // State 9:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 5, true},
                ASResult_t{0.3333333333333333, 0, 8, false},
                ASResult_t{0.33333333333333337, 0, 13, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 8, false},
                ASResult_t{0.3333333333333333, 0, 13, false},
                ASResult_t{0.33333333333333337, 0, 10, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 13, false},
                ASResult_t{0.3333333333333333, 0, 10, false},
                ASResult_t{0.33333333333333337, 0, 5, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 10, false},
                ASResult_t{0.3333333333333333, 0, 5, true},
                ASResult_t{0.33333333333333337, 0, 8, false},
            }
        },
    },

    // State 10:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 6, false},
                ASResult_t{0.3333333333333333, 0, 9, false},
                ASResult_t{0.33333333333333337, 0, 14, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 9, false},
                ASResult_t{0.3333333333333333, 0, 14, false},
                ASResult_t{0.33333333333333337, 0, 11, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 14, false},
                ASResult_t{0.3333333333333333, 0, 11, true},
                ASResult_t{0.33333333333333337, 0, 6, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 11, true},
                ASResult_t{0.3333333333333333, 0, 6, false},
                ASResult_t{0.33333333333333337, 0, 9, false},
            }
        },
    },

    // State 11:
    std::array{
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 11, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 11, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 11, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 11, true},}
        },
    },

    // State 12:
    std::array{
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 12, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 12, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 12, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 12, true},}
        },
    },

    // State 13:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 9, false},
                ASResult_t{0.3333333333333333, 0, 12, true},
                ASResult_t{0.33333333333333337, 0, 13, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 12, true},
                ASResult_t{0.3333333333333333, 0, 13, false},
                ASResult_t{0.33333333333333337, 0, 14, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 13, false},
                ASResult_t{0.3333333333333333, 0, 14, false},
                ASResult_t{0.33333333333333337, 0, 9, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 14, false},
                ASResult_t{0.3333333333333333, 0, 9, false},
                ASResult_t{0.33333333333333337, 0, 12, true},
            }
        },
    },

    // State 14:
    std::array{
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 10, false},
                ASResult_t{0.3333333333333333, 0, 13, false},
                ASResult_t{0.33333333333333337, 0, 14, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 13, false},
                ASResult_t{0.3333333333333333, 0, 14, false},
                ASResult_t{0.33333333333333337, 1, 15, true},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 0, 14, false},
                ASResult_t{0.3333333333333333, 1, 15, true},
                ASResult_t{0.33333333333333337, 0, 10, false},
            }
        },
        policy_action_t{
            0.25,
            std::vector{
                ASResult_t{0.33333333333333337, 1, 15, true},
                ASResult_t{0.3333333333333333, 0, 10, false},
                ASResult_t{0.33333333333333337, 0, 13, false},
            }
        },
    },

    // State 15:
    std::array{
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 15, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 15, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 15, true},}
        },
        policy_action_t{
            0.25,
            std::vector{ASResult_t{1.0, 0, 15, true},}
        },
    },

};

std::array<float, states.size()> evaluate_policy(const policy_t& policy, std::array<float, states.size()> V,
                                                 const float gamma,
                                                 const u32 I = 100, const float E = 0.001f)
{
    std::array<float, states.size()> Vp = V;

    for (u32 i = 0; i < I; i++)
    {
        float delta = 0;
        V = Vp;
        for (u32 x = 0; x < V.size(); x++)
        {
            auto& ps = policy[x];
            float v = 0;
            for (const auto& a : ps)
            {
                float l = 0;
                for (const auto& xp : a.action)
                {
                    l += xp.prob * (xp.reward + (gamma * V[xp.next_state]));
                }
                v += a.prob * l;
            }
            delta = std::max(delta, std::abs(V[x] - v));
            Vp[x] = v;
        }

        if (delta < E)
            break;
    }

    return Vp;
}

void print(const std::array<float, states.size()>& v)
{
    std::cout << "V: [\n";
    for (const auto& [i, x] : blt::enumerate(v))
    {
        std::cout << x << " ";
        if (i % 4 == 3)
            std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

void init(const blt::gfx::window_data&)
{
    using namespace blt::gfx;

    global_matrices.create_internals();
    resources.load_resources();
    renderer_2d.create();

    std::array<float, states.size()> V{};
    V = evaluate_policy(policy, V, 0.99);
    print(V);
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
