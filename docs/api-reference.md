---
layout: default
title: API Reference
nav_order: 7
---

# API Reference

This page summarizes the public API surface. Namespaces are omitted where they
are clearly `reflect::`.

## Client

```cpp
client(std::string_view uri);

target_dialect();
inspect_table(std::string_view table);
inspect<Model>();

transaction(function);
table<Model>();

migrate<Model>();
migrate<Model>(schema_sync_options);
migrate_force<Model>();
reset_schema<Model>();
migrate_versioned<Model>(std::string_view id);
apply_migrations(std::vector<migration>);

validate_schema<Model>();
require_schema<Model>();

insert<Model>();
insert_many<Model>();
find<Model>(id);
find_one<Model>();
find_many<Model>();
update<Model>();
update_many<Model>();
delete_many<Model>();
delete_all<Model>();
count<Model>();
exists<Model>();

execute(statement);
query(statement);
```

## Table Client

`table_client<Model>` exposes the same model-specific operations and relation
helpers:

```cpp
has_many<Related>(parent, &Related::foreign_key);
has_one<Related>(parent, &Related::foreign_key);
belongs_to<Parent>(child, &Model::foreign_key);
```

## Schema Sync Options

```cpp
struct schema_sync_options
{
    bool validate_after = true;
    bool force = false;
    schema_validation_options validation{};
};
```

`force = true` is destructive when drift is detected.

## Schema Validation Options

```cpp
struct schema_validation_options
{
    bool allow_extra_columns = false;
    bool check_types = true;
    bool strict_sql_types = false;
    bool check_nullability = true;
    bool check_primary_key = true;
    bool check_defaults = true;
    bool check_indexes = true;
    bool check_foreign_keys = true;
};
```

## Statements

```cpp
struct statement
{
    std::string sql;
    std::vector<sql_value> binds;
};
```

Use raw statements for hand-written migrations or backend-specific SQL.
