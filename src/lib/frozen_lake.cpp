/*
*  Copyright (C) 2025  Brett Terpstra
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

static blt::gfx::matrix_state_manager global_matrices;
static blt::gfx::resource_manager resources;
static blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
static blt::gfx::first_person_camera camera;

static void init(const blt::gfx::window_data&)
{
    using namespace blt::gfx;


    global_matrices.create_internals();
    resources.load_resources();
    renderer_2d.create();
}

static void update(const blt::gfx::window_data& data)
{
    global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

    camera.update();
    camera.update_view(global_matrices);
    global_matrices.update();

    renderer_2d.render(data.width, data.height);
}

static void destroy(const blt::gfx::window_data&)
{
    global_matrices.cleanup();
    resources.cleanup();
    renderer_2d.cleanup();
    blt::gfx::cleanup();
}

int main10()
{
    blt::gfx::init(blt::gfx::window_data{"Frozen Lake", init, update, destroy}.setSyncInterval(1));
}