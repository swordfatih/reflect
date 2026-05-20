#include "support/Fixtures.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("insert skips generated columns and update touches updated_at", "[statement]")
{
    const auto model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const reflect::test::User user{
        .email = "ada@example.test",
        .name = "Ada",
    };

    const auto insert = reflect::insert_statement(user, model);
    REQUIRE(insert.sql.find("\"created_at\"") == std::string::npos);
    REQUIRE(insert.sql.find("\"updated_at\"") == std::string::npos);
    REQUIRE(insert.binds.size() == 2);

    const reflect::test::User existing{
        .id = 42,
        .email = "ada@example.test",
        .name = "Ada",
    };

    const auto update = reflect::update_statement(existing, model);
    REQUIRE(update.sql.find("\"created_at\"") == std::string::npos);
    REQUIRE(update.sql.find("\"updated_at\" = CURRENT_TIMESTAMP") != std::string::npos);
    REQUIRE(update.sql.find("WHERE \"id\" = ?") != std::string::npos);
    REQUIRE(update.binds.size() == 3);
}

TEST_CASE("update_many emits guarded set clauses", "[statement]")
{
    const auto model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const reflect::test::User patch{
        .email = "new@example.test",
        .name = "New",
    };

    const auto update = reflect::update_many_statement(
        patch,
        model,
        reflect::where(&reflect::test::User::email).contains("example")
    );

    REQUIRE(update.sql.find("UPDATE \"users\" SET") != std::string::npos);
    REQUIRE(update.sql.find("\"created_at\"") == std::string::npos);
    REQUIRE(update.sql.find("WHERE \"email\" LIKE ?") != std::string::npos);
    REQUIRE(update.sql.find("\"updated_at\" = CURRENT_TIMESTAMP") != std::string::npos);
    REQUIRE(update.binds.size() == 3);
}

TEST_CASE("upsert emits primary-key conflict update", "[statement]")
{
    const reflect::test::User user{
        .id = 42,
        .email = "ada@example.test",
        .name = "Ada",
    };

    const auto sqlite_model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const auto sqlite = reflect::upsert_statement(user, sqlite_model);

    REQUIRE(sqlite.sql.find("INSERT INTO \"users\"") != std::string::npos);
    REQUIRE(sqlite.sql.find("\"id\", \"email\", \"name\"") != std::string::npos);
    REQUIRE(sqlite.sql.find("ON CONFLICT (\"id\") DO UPDATE SET") != std::string::npos);
    REQUIRE(sqlite.sql.find("\"email\" = excluded.\"email\"") != std::string::npos);
    REQUIRE(sqlite.sql.find("\"name\" = excluded.\"name\"") != std::string::npos);
    REQUIRE(sqlite.sql.find("\"updated_at\" = CURRENT_TIMESTAMP") != std::string::npos);
    REQUIRE(sqlite.sql.find("\"created_at\"") == std::string::npos);
    REQUIRE(sqlite.binds.size() == 3);

    const auto postgres_model = reflect::describe_model<reflect::test::User>(reflect::dialect::postgresql);
    const auto postgres = reflect::upsert_statement(user, postgres_model);

    REQUIRE(postgres.sql.find("ON CONFLICT (\"id\") DO UPDATE SET") != std::string::npos);
    REQUIRE(postgres.sql.find("\"updated_at\" = CURRENT_TIMESTAMP") != std::string::npos);
    REQUIRE(postgres.binds.size() == 3);
}
