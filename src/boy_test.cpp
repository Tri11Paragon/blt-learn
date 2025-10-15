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

#include <blt/logging/logging.h>
#include <blt/math/vectors.h>
#include <blt/std/random.h>
#include <blt/math/log_util.h>
#include <blt/profiling/profiler_v2.h>

using vec = blt::vec3f;
blt::random::random_t rand_v{std::random_device{}()};


struct sample_t
{
	std::array<vec, 3> n;
	std::array<vec, 3> t;
	std::array<vec, 3> bt;
	std::array<vec, 3> ws;

	sample_t()
	{
		reset();
	}

	void reset()
	{
		for (auto& v : n)
			v = get_vec();

		for (auto& v : t)
			v = get_vec();

		for (auto& v : bt)
			v = get_vec();

		for (auto& v : ws)
			v = get_vec();
	}

	static vec get_vec()
	{
		vec v;
		for (auto& f : v)
			f = rand_v.get_float();
		return v;
	}
};


struct pixel_t
{
	vec normal, tangent, bitangent, position;
};


struct weight_t
{
	float w0, w1, w2, frac;

	weight_t()
	{
		reset();
	}

	void reset()
	{
		w0   = rand_v.get_float();
		w1   = rand_v.get_float();
		w2   = rand_v.get_float();
		frac = rand_v.get_float() + 1;
	}
};


void multiply_vector(vec&        o1,
										  vec&        o2,
										  vec&        o3,
										  vec&        o4,
										  const vec&  v1,
										  const vec&  v2,
										  const vec&  v3,
										  const vec&  v4,
										  const float w)
{
	o1 += v1 * w;
	o2 += v2 * w;
	o3 += v3 * w;
	o4 += v4 * w;
}

void divide_vector(vec&        o1,
										  vec&        o2,
										  vec&        o3,
										  vec&        o4,
										  const float v)
{
	o1 /= v;
	o2 /= v;
	o3 /= v;
	o4 /= v;
}

BLT_ATTRIB_NO_INLINE void test1(pixel_t& out, const sample_t& in, const weight_t& weights)
{
	multiply_vector(out.normal,
					out.tangent,
					out.bitangent,
					out.position,
					in.n[0],
					in.t[0],
					in.bt[0],
					in.ws[0],
					weights.w0);
	multiply_vector(out.normal,
					out.tangent,
					out.bitangent,
					out.position,
					in.n[1],
					in.t[1],
					in.bt[1],
					in.ws[1],
					weights.w1);
	multiply_vector(out.normal,
					out.tangent,
					out.bitangent,
					out.position,
					in.n[2],
					in.t[2],
					in.bt[2],
					in.ws[2],
					weights.w2);
	divide_vector(out.normal,
					out.tangent,
					out.bitangent,
					out.position,
					weights.w2);
}

BLT_ATTRIB_NO_INLINE void boy_version(pixel_t& out, const sample_t& in, const weight_t& weights)
{
	out.normal    = (in.n[0] * weights.w0 + in.n[1] * weights.w1 + in.n[2] * weights.w2) / weights.frac;
	out.tangent   = (in.t[0] * weights.w0 + in.t[1] * weights.w1 + in.t[2] * weights.w2) / weights.frac;
	out.bitangent = (in.bt[0] * weights.w0 + in.bt[1] * weights.w1 + in.bt[2] * weights.w2) / weights.frac;
	out.position  = (in.ws[0] * weights.w0 + in.ws[1] * weights.w1 + in.ws[2] * weights.w2) / weights.frac;
}

int main()
{
	std::vector<pixel_t>  outputs;
	std::vector<sample_t> samples;
	std::vector<weight_t> weights;

	constexpr size_t size = 10000;
	constexpr size_t iters = 10000ul;

	outputs.resize(size);
	samples.resize(size);
	weights.resize(size);


	for (size_t _ = 0; _ < iters; _++)
	{
		for (auto& s :samples)
			s.reset();
		for (auto& w : weights)
			w.reset();
		BLT_START_INTERVAL("Profile", "Boy Version");
		for (size_t i = 0; i < size; i++) { boy_version(outputs[i], samples[i], weights[i]); }
		BLT_END_INTERVAL("Profile", "Boy Version");
		blt::black_box(outputs);
	}


	for (size_t _ = 0; _ < iters; _++)
	{
		for (auto& s :samples)
			s.reset();
		for (auto& w : weights)
			w.reset();
		BLT_START_INTERVAL("Profile", "Test1");
		for (size_t i = 0; i < size; i++) { test1(outputs[i], samples[i], weights[i]); }
		BLT_END_INTERVAL("Profile", "Test1");
		blt::black_box(outputs);
	}

	BLT_PRINT_PROFILE("Profile");
}
