#include "support/Fixtures.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("sqlite backend enforces declared foreign keys", "[sqlite][constraints]")
{
    reflect::client db{"sqlite://:memory:"};
    db.migrate<reflect::test::User>();
    db.migrate<reflect::test::Post>();

    REQUIRE_THROWS_AS(
        db.insert<reflect::test::Post>({
            .user_id = 404,
            .title = "orphaned post",
        }),
        reflect::foreign_key_violation
    );
}

TEST_CASE("sqlite migrations roll back failed transactional batches", "[sqlite][migration]")
{
    reflect::client db{"sqlite://:memory:"};

    REQUIRE_THROWS_AS(
        db.apply_migrations({
            reflect::migration{
                .id = "001_failed",
                .statements = {
                    reflect::statement{.sql = "CREATE TABLE \"rolled_back\" (\"id\" INTEGER PRIMARY KEY)"},
                    reflect::statement{.sql = "THIS IS NOT SQL"},
                },
            },
        }),
        reflect::migration_error
    );

    REQUIRE_FALSE(db.inspect_table("rolled_back").exists());
}
