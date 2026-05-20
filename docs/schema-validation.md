---
layout: default
title: Schema Validation
nav_order: 5
---

# Schema Validation

Validation compares the reflected C++ model against the actual database table.
It is the production guard that prevents Reflect from silently accepting drift.

```cpp
db.require_schema<User>();
```

or:

```cpp
auto result = db.validate_schema<User>();
if(!result.valid())
{
    for(const auto& issue: result.issues)
    {
        // inspect issue.kind, issue.object_name, issue.expected, issue.actual
    }
}
```

## What Is Checked

- table exists
- missing model columns
- extra database columns
- SQL type compatibility
- nullability
- primary key status
- default expressions
- expected indexes and unique indexes
- foreign keys and referential actions

## Validation Options

```cpp
db.require_schema<User>({
    .allow_extra_columns = false,
    .check_types = true,
    .strict_sql_types = false,
    .check_nullability = true,
    .check_primary_key = true,
    .check_defaults = true,
    .check_indexes = true,
    .check_foreign_keys = true,
});
```

By default, Reflect checks type families. This avoids false positives where a
dialect reports equivalent types differently, such as SQLite `TEXT` for model
`VARCHAR(320)`.

Use strict SQL types when exact declarations matter:

```cpp
db.require_schema<User>({
    .strict_sql_types = true,
});
```

## Failure Mode

`require_schema<T>()` throws `reflect::schema_validation_error`. The exception
contains the full `schema_validation_result`, so callers can log structured
drift data.
