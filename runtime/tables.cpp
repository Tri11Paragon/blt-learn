/*
 *  <Short Description>
 *  Copyright (C) 2026  Brett Terpstra
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

#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <blt/std/string.h>
#include <blt/meta/meta.h>

#include "blt/iterator/enumerate.h"
#include "blt/logging/logging.h"

namespace blt::table
{
    namespace detail
    {
        BLT_META_MAKE_FUNCTION_CHECK(str);

        BLT_META_MAKE_FUNCTION_CHECK(string);

        BLT_META_MAKE_FUNCTION_CHECK(to_string);
    }

    struct column_t
    {
        std::optional<std::string> name;
    };

    struct row_t
    {
        std::optional<std::string> name;
        std::vector<std::string> data;

        row_t(std::string name, std::vector<std::string> data) :
            name(std::move(name)), data(std::move(data))
        {
        }

        explicit row_t(std::string name) : name(std::move(name))
        {
        }

        row_t& add(std::string value)
        {
            data.push_back(std::move(value));
            return *this;
        }

        template <typename T>
        row_t& add(T&& t)
        {
            if constexpr (detail::has_func_str_v<T>)
            {
                return add(std::forward<T>(t).str());
            }
            else if constexpr (detail::has_func_string_v<T>)
            {
                return add(std::forward<T>(t).string());
            }
            else if constexpr (detail::has_func_to_string_v<T>)
            {
                return add(std::forward<T>(t).to_string());
            }
            else if constexpr (std::is_arithmetic_v<T>)
            {
                return add(std::to_string(std::forward<T>(t)));
            }
            else
            {
                std::stringstream s;
                s << std::forward<T>(t);
                return add(s.str());
            }
        }
    };

    struct table_data_t
    {
        struct config_t
        {
            table_data_t* table;

            explicit config_t(table_data_t& table) : table(&table)
            {
            }

            // automatically generates columns with empty names when inserting rows
            // note: this won't insert a column for the row's name
            config_t& auto_column()
            {
                table->automatically_generate_columns = true;
                return *this;
            }

            [[nodiscard]] table_data_t& build() const
            {
                return *table;
            }
        };

        table_data_t() = default;

        config_t config()
        {
            return config_t{*this};
        }

        table_data_t& named_rows(const std::string& corner = "")
        {
            columns.insert(columns.begin(), column_t{corner});
            return *this;
        }

        table_data_t& add_column(const std::string& name)
        {
            columns.push_back(column_t{name});
            return *this;
        }

        table_data_t& add_row(row_t row)
        {
            if (automatically_generate_columns)
            {
                if (row.name.has_value() && !has_inserted_named_column)
                {
                    named_rows();
                    has_inserted_named_column = true;
                }
                for (size_t i = columns.size(); i < row.data.size(); i++)
                    columns.emplace_back();
            }
            rows.push_back(std::move(row));
            return *this;
        }

        std::vector<column_t> columns;
        std::vector<row_t> rows;

        bool automatically_generate_columns = false;
        bool has_inserted_named_column = false;
    };

    // helper function enables blt::table::build();
    inline table_data_t build()
    {
        return table_data_t{};
    }

    struct latex_table_formatter_t
    {
        struct column_formatter_t
        {
            std::string left_seperator = "|";
            std::string right_seperator = "|";

            column_formatter_t& set_left_seperator(const char l)
            {
                left_seperator = l;
                return *this;
            }

            column_formatter_t& set_right_seperator(const char r)
            {
                right_seperator = r;
                return *this;
            }
        };

        struct config_t
        {
            latex_table_formatter_t* format;

            explicit config_t(latex_table_formatter_t& format) : format(&format)
            {
            }

            [[nodiscard]] config_t single_ruled() const
            {
                format->double_headers = false;
                return *this;
            }

            [[nodiscard]] config_t open_rows() const
            {
                format->separate_rows = false;
                return *this;
            }

            [[nodiscard]] latex_table_formatter_t& build() const
            {
                return *format;
            }
        };

        latex_table_formatter_t() = default;

        std::string format(const table_data_t& table)
        {
        }

        bool separate_rows = true;
        bool double_headers = true;
        std::optional<column_formatter_t> column_separator = column_formatter_t{};
    };
}

int main()
{
}
