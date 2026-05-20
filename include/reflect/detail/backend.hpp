#pragma once

#include <reflect/dialect.hpp>
#include <reflect/introspection.hpp>
#include <reflect/value.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace reflect
{

struct statement;

} // namespace reflect

namespace reflect::detail
{

using row = std::vector<sql_value>;

struct execution_result
{
    std::uint64_t rows_affected = 0;
    std::int64_t  last_insert_id = 0;
};

class backend
{
public:
    virtual ~backend() = default;

    [[nodiscard]] virtual dialect target_dialect() const noexcept = 0;
    virtual execution_result execute(const reflect::statement& input) = 0;
    [[nodiscard]] virtual std::vector<row> query(const reflect::statement& input) = 0;
    [[nodiscard]] virtual std::vector<std::string> table_columns(std::string_view table) = 0;
    [[nodiscard]] virtual reflect::table_info inspect_table(std::string_view table) = 0;
};

std::unique_ptr<backend> make_sqlite_backend(std::string path);
std::unique_ptr<backend> make_postgresql_backend(std::string connection_info);

} // namespace reflect::detail
