# reflect

`reflect` is a C++26 reflection-first ORM. Define normal aggregate structs, add
small annotations, and Reflect derives table metadata, DDL, type-safe filters,
CRUD statements, migrations, validation, and row materialization.

It is designed to feel familiar if you have used an ORM before, while staying idiomatic to
C++: the C++ model is the source of truth.

## Status

Reflect is pre-1.0. It is usable for experiments and early applications, but the
stable V1 line still needs packaging, more migration tooling, and wider backend
coverage.

## Example

```cpp
#include <reflect/reflect.hpp>

#include <cstdint>
#include <string>

struct [[= reflect::table{"users"}]] User
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::unique, = reflect::indexed, = reflect::not_null, = reflect::varchar{320}]]
    std::string email;

    [[= reflect::not_null, = reflect::varchar{120}]]
    std::string name;
};

int main()
{
    reflect::client db{"sqlite://:memory:"};

    db.migrate<User>();
    db.require_schema<User>();

    db.insert<User>({.email = "ada@example.test", .name = "Ada"});

    auto users = db.find_many<User>(
        reflect::query<User>(reflect::where(&User::email).ends_with("@example.test"))
            .order_by(&User::name)
            .take(25)
    );
}
```

## Core Features

- C++26 static reflection model discovery.
- SQLite and PostgreSQL backends.
- Model annotations for table names, column names, primary keys, auto increment,
  unique columns, indexes, nullability, defaults, checks, foreign keys, varchar,
  decimal, text, JSON, UUID, blob, date, time, and timestamp fields.
- Generated schema DDL.
- CRUD helpers: `insert`, `insert_many`, `find`, `find_one`, `find_many`,
  `update`, `update_many`, `delete_many`, `delete_all`, `count`, and `exists`.
- Query builder with typed field predicates and bound parameters.
- Relationship helpers: `has_many`, `has_one`, and `belongs_to`.
- Transactions with nested savepoints.
- Runtime table introspection.
- Schema validation and drift reporting.
- Additive schema sync and versioned manual migrations.
- Opt-in destructive development reset for fast iteration.

## Schema Validation

Reflect can compare your C++ model against the actual database table:

```cpp
auto result = db.validate_schema<User>();

if(!result.valid())
{
    for(const auto& issue: result.issues)
    {
        // missing_column, type_mismatch, nullability_mismatch, etc.
    }
}
```

For startup checks, prefer the throwing helper:

```cpp
db.require_schema<User>();
```

Validation checks:

- table exists
- missing columns
- extra columns
- SQL type compatibility
- nullability
- primary key status
- default expressions
- indexes and unique indexes
- foreign keys and referential actions

By default, validation allows equivalent type families such as SQLite `TEXT`
versus model `VARCHAR(320)`. Use strict type checking when exact SQL text
matters:

```cpp
db.require_schema<User>({
    .strict_sql_types = true,
});
```

## Migrations

Reflect has two migration styles.

### Additive Sync

```cpp
db.migrate<User>();
```

This creates the table if missing, adds missing columns, and creates expected
indexes. It is intentionally conservative: it does not drop columns, rename
columns, change column types, or rewrite constraints.

After syncing, Reflect validates the table by default. If the existing table has
drift that additive sync cannot fix safely, it reports the mismatch instead of
silently accepting it.

### Versioned Manual Migrations

```cpp
db.apply_migrations({
    reflect::migration{
        .id = "002_add_user_search_index",
        .statements = {
            reflect::statement{
                .sql = "CREATE INDEX IF NOT EXISTS \"idx_users_name\" ON \"users\" (\"name\")",
            },
        },
    },
});
```

Versioned migrations are recorded in `reflect_schema_migrations`. They are
transactional by default and skipped when their ID has already been applied.

Compared with mature ORMs, this is useful but not complete. It covers first
schema creation, additive changes, and explicit hand-written SQL migrations. It
does not yet generate reversible migration files, detect drift across a whole
database, or produce automatic table rebuild plans.

## Destructive Development Mode

For local development, Reflect can drop and recreate a table when validation
finds drift:

```cpp
db.migrate<User>({
    .force = true,
});
```

or:

```cpp
db.migrate_force<User>();
db.reset_schema<User>();
```

These APIs are destructive. Use them for local iteration, tests, demos, or
throwaway databases. Do not use them against production data unless data loss is
intended and backed up.

## Relationships

Declare foreign keys with `reflect::references`:

```cpp
struct [[= reflect::table{"posts"}]] Post
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"users", "id", "CASCADE"}]]
    std::int64_t user_id = 0;
};
```

Then load related records:

```cpp
auto user_posts = db.table<User>().has_many<Post>(user, &Post::user_id);
auto author = db.table<Post>().belongs_to<User>(post, &Post::user_id);
```

## Examples

- `examples/playground.cpp`: minimal SQLite usage.
- `examples/production_blog.cpp`: richer model set with JSON, dates,
  relations, transactions, and manual migrations.
- `examples/migrations_sqlite.cpp`: migration-focused example using the SQLite
  backend directly, legacy table drift, validation, additive migration, and
  destructive development reset.

## Documentation

The GitHub Pages documentation lives in `docs/` and uses Just the Docs. Start at
`docs/index.md` or the published project page.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Requirements:

- CMake 3.30+
- a compiler with C++26 static reflection support
- `-freflection`

## What Is Still Missing For A Complete V1

- Generated migration files with up/down support.
- Database-wide drift detection.
- Safe table rebuild plans for SQLite column changes.
- Column rename/drop/type-change workflows.
- Relation-aware joins, eager loading, nested includes, and many-to-many helpers.
- Partial update objects that do not require full model instances.
- Connection pooling, async APIs, statement caching, retries, and query timeouts.
- Query logging, tracing, metrics, and slow-query hooks.
- A CLI for migration generation/application, inspection, and seeding.
- Packaging/install rules and a published compiler/database compatibility matrix.
