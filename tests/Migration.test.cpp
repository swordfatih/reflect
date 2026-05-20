#include "support/Fixtures.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("migration planning creates schema for missing tables", "[migration]")
{
    const auto model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const auto plan = reflect::plan_migration(model, {});

    REQUIRE(!plan.empty());
    REQUIRE(plan.statements.front().sql.find("CREATE TABLE IF NOT EXISTS \"users\"") != std::string::npos);
}

TEST_CASE("migration planning adds missing columns for existing tables", "[migration]")
{
    const auto model = reflect::describe_model<reflect::test::User>(reflect::dialect::sqlite);
    const auto plan = reflect::plan_migration(model, std::vector<std::string>{"id", "email"});

    bool found_name_column = false;
    for(const auto& statement: plan.statements)
    {
        found_name_column = found_name_column || statement.sql.find("ADD COLUMN \"name\"") != std::string::npos;
    }

    REQUIRE(found_name_column);
}

TEST_CASE("versioned migration metadata has stable table contract", "[migration]")
{
    const auto create = reflect::create_migration_table_statement();
    REQUIRE(create.sql.find("\"reflect_schema_migrations\"") != std::string::npos);
    REQUIRE(create.sql.find("\"id\" TEXT PRIMARY KEY") != std::string::npos);

    const auto record = reflect::record_migration_statement("001_initial");
    REQUIRE(record.sql.find("INSERT INTO \"reflect_schema_migrations\"") != std::string::npos);
    REQUIRE(record.binds.size() == 1);
}

TEST_CASE("migrations are transactional by default", "[migration]")
{
    const reflect::migration migration{
        .id = "001_initial",
        .statements = {},
    };

    REQUIRE(migration.transactional);
}
