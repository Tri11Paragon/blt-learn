#pragma once
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

#ifndef BLT_NMIST_LOADER_H
#define BLT_NMIST_LOADER_H

#include <blt/std/types.h>
#include <vector>
#include <variant>
#include <fstream>

#include <blt/iterator/iterator.h>
#include <blt/std/ranges.h>

#include <blt/std/memory_util.h>

#include <blt/std/assert.h>

namespace blt::fs
{
    class idx_file_t
    {
        template <typename T>
        using mk_v = std::vector<T>;
        using vec_t = std::variant<mk_v<blt::u8>, mk_v<blt::i8>, mk_v<blt::u16>, mk_v<blt::u32>, mk_v<blt::f32>, mk_v<
                                       blt::f64>>;

        std::vector<blt::u32> dimensions;
        vec_t data;

        template <typename T>
        void read_data(std::ifstream& file, blt::size_t total_size)
        {
            auto& array = std::get<mk_v<T>>(data);
            array.resize(total_size);
            file.read(reinterpret_cast<char*>(array.data()), static_cast<std::streamsize>(total_size) * sizeof(T));
        }

        template <typename T>
        void reverse_data()
        {
            auto& array = std::get<mk_v<T>>(data);
            for (auto& v : array)
                blt::mem::reverse(v);
        }

    public:
        explicit idx_file_t(const std::string& path)
        {
            std::ifstream file{path, std::ios::in | std::ios::binary};

            using char_type = std::ifstream::char_type;
            char_type magic_arr[4];
            file.read(magic_arr, 4);
            BLT_ASSERT(magic_arr[0] == 0 && magic_arr[1] == 0);

            blt::u8 dims = magic_arr[3];
            blt::size_t total_size = 1;

            for (blt::i32 i = 0; i < dims; i++)
            {
                char_type dim_arr[4];
                file.read(dim_arr, 4);
                blt::u32 dim;
                blt::mem::fromBytes(dim_arr, dim);
                dimensions.push_back(dim);
                total_size *= dim;
            }

            switch (magic_arr[2])
            {
            // unsigned char
            case 0x08:
                data = mk_v<blt::u8>{};
                read_data<blt::u8>(file, total_size);
                break;
            // signed char
            case 0x09:
                data = mk_v<blt::i8>{};
                read_data<blt::i8>(file, total_size);
                break;
            // short
            case 0x0B:
                data = mk_v<blt::u16>{};
                read_data<blt::u16>(file, total_size);
                reverse_data<blt::u16>();
                break;
            // int
            case 0x0C:
                data = mk_v<blt::u32>{};
                read_data<blt::u32>(file, total_size);
                reverse_data<blt::u32>();
                break;
            // float
            case 0x0D:
                data = mk_v<blt::f32>{};
                read_data<blt::f32>(file, total_size);
                reverse_data<blt::f32>();
                break;
            // double
            case 0x0E:
                data = mk_v<blt::f64>{};
                read_data<blt::f64>(file, total_size);
                reverse_data<blt::f64>();
                break;
            default:
                BLT_ERROR("Unspported idx file type!");
            }
            if (file.eof())
            {
                BLT_ERROR("EOF reached. It's unlikely your file was read correctly!");
            }
        }

        template <typename T>
        [[nodiscard]] const std::vector<T>& get_data_as() const
        {
            return std::get<mk_v<T>>(data);
        }

        template <typename From, typename To>
        [[nodiscard]] std::vector<To> cast_to() const
        {
            if (!std::holds_alternative<mk_v<From>>(data))
                BLT_ABORT("Cannot cast to requested type. Data is not of type");
            auto vec = std::get<mk_v<From>>(data);
            std::vector<To> ret;
            ret.reserve(vec.size());
            for (const auto& v : vec)
                ret.push_back(static_cast<To>(v));
            return ret;
        }

        template <typename T>
        std::vector<span<T>> get_as_spans() const
        {
            std::vector<span<T>> spans;

            size_t total_size = data_size(1);

            for (size_t i = 0; i < dimensions[0]; i++)
            {
                auto& array = std::get<mk_v<T>>(data);
                spans.push_back({&array[i * total_size], total_size});
            }

            return spans;
        }

        [[nodiscard]] const std::vector<blt::u32>& get_dimensions() const
        {
            return dimensions;
        }

        [[nodiscard]] blt::size_t data_size(const blt::size_t starting_dimension = 0) const
        {
            blt::size_t total_size = 1;
            for (const auto d : blt::iterate(dimensions).skip(starting_dimension))
                total_size *= d;
            return total_size;
        }
    };
}

#endif //BLT_NMIST_LOADER_H
