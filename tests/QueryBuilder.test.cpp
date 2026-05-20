#include "support/Fixtures.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("query builder emits filtering sorting and pagination", "[query]")
{
    const auto model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const auto query =
        reflect::query<reflect::test::User>(reflect::where(&reflect::test::User::email).ends_with("@test.com"))
            .order_by(&reflect::test::User::name)
            .take(10)
            .skip(20);

    const auto statement = reflect::select_statement(model, query.options());

    REQUIRE(statement.sql.find("WHERE \"email\" LIKE ?") != std::string::npos);
    REQUIRE(statement.sql.find("ORDER BY \"name\" ASC") != std::string::npos);
    REQUIRE(statement.sql.find("LIMIT 10") != std::string::npos);
    REQUIRE(statement.sql.find("OFFSET 20") != std::string::npos);
    REQUIRE(statement.binds.size() == 1);
}

TEST_CASE("field predicates support in and between", "[query]")
{
    const auto in_filter = reflect::where(&reflect::test::User::id).in({1, 2, 3});
    REQUIRE(in_filter.sql == "\"id\" IN (?, ?, ?)");
    REQUIRE(in_filter.binds.size() == 3);

    const auto between_filter = reflect::where(&reflect::test::User::id).between(10, 20);
    REQUIRE(between_filter.sql == "\"id\" BETWEEN ? AND ?");
    REQUIRE(between_filter.binds.size() == 2);
}

TEST_CASE("count and exists statements use compact SQL", "[query]")
{
    const auto model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const auto filter = reflect::where(&reflect::test::User::email).contains("example");

    const auto count = reflect::count_statement(model, filter);
    REQUIRE(count.sql.find("SELECT COUNT(*) FROM \"users\" WHERE") != std::string::npos);

    const auto exists = reflect::exists_statement(model, filter);
    REQUIRE(exists.sql.find("SELECT 1 FROM \"users\" WHERE") != std::string::npos);
    REQUIRE(exists.sql.find("LIMIT 1") != std::string::npos);
}
