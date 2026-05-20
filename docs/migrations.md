---
layout: default
title: Migrations
nav_order: 6
---

# Migrations

Reflect deliberately separates validation, conservative migration, and
destructive development reset.

## Additive Sync

```cpp
db.migrate<User>();
```

Additive sync:

- creates the table if missing
- adds missing model columns
- creates expected indexes
- validates the schema after applying the plan

It does not drop columns, rename columns, change types, rewrite constraints, or
delete data.

## Versioned Manual Migrations

```cpp
db.apply_migrations({
    reflect::migration{
        .id = "001_add_search_index",
        .statements = {
            reflect::statement{
                .sql = "CREATE INDEX IF NOT EXISTS \"idx_users_name\" ON \"users\" (\"name\")",
            },
        },
    },
});
```

Applied migration IDs are stored in `reflect_schema_migrations`. Migrations are
transactional by default.

## Destructive Development Reset

Use force mode only for development databases, tests, and demos:

```cpp
db.migrate<User>({
    .force = true,
});
```

If the table exists and validation reports drift, Reflect drops and recreates
the table.

You can also force a reset directly:

```cpp
db.reset_schema<User>();
```

{: .warning }
These APIs can destroy data. Do not use them against production databases unless
data loss is intended.

## How This Compares To Mature ORMs

Reflect's current migration system is useful for:

- first schema creation
- additive local changes
- hand-written versioned SQL migrations
- startup drift checks
- development resets

It is not yet equivalent to TypeORM, Sequelize, Diesel, SeaORM, or SQLx
migration workflows. Missing pieces include generated migration files, down
migrations, whole-database drift detection, column rename/drop/type-change
planning, and CLI-driven migration application.
