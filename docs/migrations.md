---
layout: default
title: Migrations
nav_order: 7
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

```cpp
reflect::migration{
    .id = "003_non_transactional_maintenance",
    .statements = {
        reflect::statement{.sql = "..."},
    },
    .transactional = false,
};
```

When `apply_migrations` is called inside `client::transaction`, Reflect disables
the inner migration transaction and lets the outer transaction control commit or
rollback.

## Versioned Model Sync

```cpp
db.migrate_versioned<User>("001_users");
```

This creates the same additive plan as `migrate<User>()`, records the migration
ID if it ran, and validates the table afterward. If the migration ID is already
recorded, it skips statements and still validates the table.

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

## Migration Planning API

For tooling, you can build the generated plan without applying it:

```cpp
auto model = reflect::describe_model<User>(reflect::dialect::sqlite);
auto plan = reflect::plan_migration(model, existing_column_names);

for(const auto& statement: plan.statements)
{
    // inspect statement.sql and statement.binds
}
```

`plan_migration` only knows existing column names. Full drift details come from
schema validation.
