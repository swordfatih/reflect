#include <reflect/reflect.hpp>

#include <chrono>
#include <cstdint>
#include <string>

#include <spdlog/spdlog.h>

struct [[= reflect::table{"users"}]] User
{
    [[= reflect::primary_key, = reflect::auto_increment]]
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

int main()
{
    spdlog::info("started");

    auto db = reflect::client("sqlite://:memory:");
    auto users = db.table<User>();
    users.migrate();

    const auto ada = db.insert<User>({.email = "ada@test.com", .name = "Ada"});
    const auto grace = users.insert({.email = "grace@test.com", .name = "Grace"});
    spdlog::info("inserted rows: {}, {}", ada.rows_affected, grace.rows_affected);

    const auto matches = users.find_many(
        reflect::where(&User::email).ends_with("@test.com")
    );

    for(const auto& user: matches)
    {
        spdlog::info("user {}: {} <{}>", user.id, user.name, user.email);
    }
}
