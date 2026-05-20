#include <reflect/detail/backend.hpp>
#include <reflect/error.hpp>
#include <reflect/statement.hpp>

#include <sl3.hpp>

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace reflect::detail
{
namespace
{

[[nodiscard]] sl3::DbValue to_sl3_value(const sql_value& value)
{
    return std::visit(
        [](const auto& current) -> sl3::DbValue {
            using current_type = std::remove_cvref_t<decltype(current)>;

            if constexpr(std::same_as<current_type, std::monostate>)
            {
                return sl3::DbValue{sl3::Type::Null};
            }
            else if constexpr(std::same_as<current_type, bool>)
            {
                return sl3::DbValue{static_cast<std::int64_t>(current ? 1 : 0)};
            }
            else if constexpr(std::same_as<current_type, std::int64_t>)
            {
                return sl3::DbValue{current};
            }
            else if constexpr(std::same_as<current_type, std::uint64_t>)
            {
                if(current > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
                {
                    throw std::overflow_error{"SQLite INTEGER cannot store this uint64_t value"};
                }

                return sl3::DbValue{static_cast<std::int64_t>(current)};
            }
            else if constexpr(std::same_as<current_type, double>)
            {
                return sl3::DbValue{current};
            }
            else if constexpr(std::same_as<current_type, std::string>)
            {
                return sl3::DbValue{current};
            }
            else
            {
                return sl3::DbValue{sl3::Blob{current.begin(), current.end()}};
            }
        },
        value
    );
}

[[nodiscard]] sl3::DbValues to_sl3_values(const std::vector<sql_value>& values)
{
    std::vector<sl3::DbValue> converted;
    converted.reserve(values.size());

    for(const auto& value: values)
    {
        converted.emplace_back(to_sl3_value(value));
    }

    return sl3::DbValues{std::move(converted)};
}

[[nodiscard]] sql_value from_sl3_column(const sl3::Columns& columns, int index)
{
    switch(columns.getType(index))
    {
        case sl3::Type::Null:
            return std::monostate{};

        case sl3::Type::Int:
            return static_cast<std::int64_t>(columns.getInt64(index));

        case sl3::Type::Real:
            return columns.getReal(index);

        case sl3::Type::Text:
            return columns.getText(index);

        case sl3::Type::Blob:
            return bytes{columns.getBlob(index)};

        case sl3::Type::Variant:
            break;
    }

    return columns.getText(index);
}

} // namespace

class sqlite_backend final : public backend
{
public:
    explicit sqlite_backend(std::string path)
        : database_{std::move(path)}
    {
    }

    [[nodiscard]] dialect target_dialect() const noexcept override
    {
        return dialect::sqlite;
    }

    execution_result execute(const reflect::statement& input) override
    {
        try
        {
            auto command = database_.prepare(input.sql);
            command.execute(to_sl3_values(input.binds));

            return execution_result{
                .rows_affected = database_.getRecentlyChanged(),
                .last_insert_id = database_.getLastInsertRowid(),
            };
        }
        catch(const std::exception& error)
        {
            throw_classified_sql_error("SQLite", "execute", input.sql, error.what());
        }
    }

    [[nodiscard]] std::vector<row> query(const reflect::statement& input) override
    {
        try
        {
            std::vector<row> rows;
            auto             command = database_.prepare(input.sql);

            command.execute(
                [&](sl3::Columns columns) {
                    row output;
                    output.reserve(static_cast<std::size_t>(columns.count()));

                    for(int index = 0; index < columns.count(); ++index)
                    {
                        output.emplace_back(from_sl3_column(columns, index));
                    }

                    rows.emplace_back(std::move(output));
                    return true;
                },
                to_sl3_values(input.binds)
            );

            return rows;
        }
        catch(const std::exception& error)
        {
            throw_classified_sql_error("SQLite", "query", input.sql, error.what());
        }
    }

    [[nodiscard]] std::vector<std::string> table_columns(std::string_view table) override
    {
        statement input;
        input.sql = "PRAGMA table_info(" + quote_identifier(table) + ")";

        std::vector<std::string> output;
        for(const auto& result_row: query(input))
        {
            if(result_row.size() > 1)
            {
                output.emplace_back(value_to_string(result_row[1]));
            }
        }

        return output;
    }

    [[nodiscard]] table_info inspect_table(std::string_view table) override
    {
        table_info output;
        output.name = std::string{table};

        statement columns_statement;
        columns_statement.sql = "PRAGMA table_info(" + quote_identifier(table) + ")";

        for(const auto& result_row: query(columns_statement))
        {
            if(result_row.size() < 6)
            {
                continue;
            }

            output.columns.emplace_back(column_info{
                .name = value_to_string(result_row[1]),
                .sql_type = value_to_string(result_row[2]),
                .nullable = value_to_i64(result_row[3]) == 0,
                .primary_key = value_to_i64(result_row[5]) != 0,
                .default_sql = value_to_string(result_row[4]),
            });
        }

        statement indexes_statement;
        indexes_statement.sql = "PRAGMA index_list(" + quote_identifier(table) + ")";

        for(const auto& result_row: query(indexes_statement))
        {
            if(result_row.size() < 3)
            {
                continue;
            }

            index_info index{
                .name = value_to_string(result_row[1]),
                .unique = value_to_i64(result_row[2]) != 0,
            };

            statement index_columns_statement;
            index_columns_statement.sql = "PRAGMA index_info(" + quote_identifier(index.name) + ")";

            for(const auto& index_row: query(index_columns_statement))
            {
                if(index_row.size() > 2)
                {
                    index.columns.emplace_back(value_to_string(index_row[2]));
                }
            }

            output.indexes.emplace_back(std::move(index));
        }

        statement foreign_keys_statement;
        foreign_keys_statement.sql = "PRAGMA foreign_key_list(" + quote_identifier(table) + ")";

        for(const auto& result_row: query(foreign_keys_statement))
        {
            if(result_row.size() < 7)
            {
                continue;
            }

            output.foreign_keys.emplace_back(foreign_key_info{
                .column = value_to_string(result_row[3]),
                .referenced_table = value_to_string(result_row[2]),
                .referenced_column = value_to_string(result_row[4]),
                .on_update = value_to_string(result_row[5]),
                .on_delete = value_to_string(result_row[6]),
            });
        }

        return output;
    }

private:
    sl3::Database database_;
};

std::unique_ptr<backend> make_sqlite_backend(std::string path)
{
    return std::make_unique<sqlite_backend>(std::move(path));
}

} // namespace reflect::detail
