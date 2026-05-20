#include <catch2/catch_test_macros.hpp>

#include <reflect/reflect.hpp>

#include <chrono>
#include <string>
#include <variant>

TEST_CASE("chrono values bind and materialize as SQL text", "[value]")
{
    const auto day = std::chrono::sys_days{std::chrono::year{2026} / 5 / 20};
    const auto date_value = reflect::detail::to_sql_value(day);

    REQUIRE(std::get<std::string>(date_value) == "2026-05-20");

    std::chrono::sys_days parsed{};
    reflect::detail::assign_sql_value(date_value, parsed);
    REQUIRE(parsed == day);

    const auto seconds = std::chrono::hours{15} + std::chrono::minutes{30} + std::chrono::seconds{5};
    const auto time_value = reflect::detail::to_sql_value(seconds);
    REQUIRE(std::get<std::string>(time_value) == "15:30:05");
}

TEST_CASE("annotation text is structural and viewable", "[annotations]")
{
    constexpr reflect::table table{"users"};
    static_assert(table.name.view() == "users");
    REQUIRE(table.name.view() == "users");
}
