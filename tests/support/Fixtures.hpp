#pragma once

#include <reflect/reflect.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace reflect::test
{

struct [[= reflect::table{"users"}]] User
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::unique, = reflect::indexed, = reflect::not_null, = reflect::varchar{320}]]
    std::string email;

    [[= reflect::not_null, = reflect::varchar{120}]]
    std::string name;

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};

    [[= reflect::updated_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point updated_at{};
};

struct [[= reflect::table{"posts"}]] Post
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"users", "id", "CASCADE", "CASCADE"}]]
    std::int64_t user_id = 0;

    [[= reflect::not_null, = reflect::varchar{200}]]
    std::string title;
};

inline const column_descriptor& column_by_name(const model_descriptor<User>& model, std::string_view name)
{
    const auto found = std::ranges::find_if(model.columns, [&](const column_descriptor& column) {
        return column.name == name;
    });

    if(found == model.columns.end())
    {
        throw std::logic_error{"missing fixture column"};
    }

    return *found;
}

} // namespace reflect::test
