/*
 *  <Short Description>
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
#include <blt/math/colors.h>
#include <imgui.h>
#include <complex>

using F = std::complex<double>;

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager     resources;
blt::gfx::batch_renderer_2d    renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera  camera;

constexpr size_t WIDTH  = 1600;
constexpr size_t HEIGHT = 900;

template<size_t Grad>
auto make_colors()
{
	std::array<blt::vec3, Grad> colors;
	const float step = 1.0f / static_cast<float>(Grad);
	for (size_t i = 0; i < Grad; i++)
		colors[i] = blt::color_t{blt::color::from<blt::color::hsv_t>(blt::vec3{static_cast<float>(i) * step * 360, 1, 1})}.as_srgb().to_vec3();
	return colors;
}

auto colors = make_colors<20>();

std::array<blt::vec3, WIDTH * HEIGHT> buffer;

size_t mandle(const F c, const double thresh = 300, F z = {0, 0})
{
	constexpr size_t iterations = 1000;
	for (size_t i = 0; i < iterations; i++)
	{
		z = z * z + c;
		if (std::abs(z) > thresh) { return i; }
	}
	return iterations;
}

void init(const blt::gfx::window_data&)
{
	using namespace blt::gfx;

	auto* buffer_texture = new texture_gl2D(WIDTH, HEIGHT);
	resources.set("buffer", buffer_texture);

	for (size_t i = 0; i < WIDTH; i++)
	{
		for (size_t j = 0; j < HEIGHT; j++)
		{
			const auto index = mandle(F{
				static_cast<double>(i) / static_cast<double>(WIDTH) * 2 - 2,
				static_cast<double>(j) / static_cast<double>(HEIGHT) * 2 - 1});
			::buffer[j * WIDTH + i] = colors[index % colors.size()];
		}
	}
	buffer_texture->upload(::buffer.data(), WIDTH, HEIGHT, GL_RGB, GL_FLOAT);

	global_matrices.create_internals();
	resources.load_resources();
	renderer_2d.create();
}

void update(const blt::gfx::window_data& data)
{
	using namespace blt::gfx;
	global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

	camera.update();
	camera.update_view(global_matrices);
	global_matrices.update();

	renderer_2d.drawRectangle(rectangle2d_t{WIDTH / 2., HEIGHT / 2., WIDTH, HEIGHT}, "buffer");

	renderer_2d.render(data.width, data.height);
}

void destroy(const blt::gfx::window_data&)
{
	global_matrices.cleanup();
	resources.cleanup();
	renderer_2d.cleanup();
	blt::gfx::cleanup();
}

int main() { blt::gfx::init(blt::gfx::window_data{"My Sexy Window", init, update, destroy}.setSyncInterval(1)); }
