---
layout: default
title: Models
nav_order: 3
---

# Models

Models are ordinary aggregate structs. Reflect reads table metadata from C++26
static reflection and field annotations.

```cpp
struct [[= reflect::table{"posts"}]] Post
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"users", "id", "CASCADE"}]]
    std::int64_t user_id = 0;

    [[= reflect::not_null, = reflect::varchar{200}]]
    std::string title;

    [[= reflect::nullable, = reflect::text]]
    std::optional<std::string> summary;

    [[= reflect::json, = reflect::not_null, = reflect::default_value{"'{}'"}]]
    std::string metadata_json = "{}";
};
```

## Table And Column Names

```cpp
[[= reflect::table{"users"}]]
[[= reflect::column{"email_address"}]]
```

Without explicit names, Reflect uses the C++ type and field names.

Annotation strings are compile-time values with a 256-character maximum.

Aliases:

- `reflect::map` is an alias for `reflect::column`
- `reflect::db_type` is an alias for `reflect::sql_type`
- `reflect::default_sql` is an alias for `reflect::default_value`
- `reflect::numeric` is an alias for `reflect::decimal`
- `reflect::index` is an alias for `reflect::indexed`
- `reflect::ignored` is an alias for `reflect::ignore`

## Keys And Indexes

```cpp
[[= reflect::id]]
[[= reflect::auto_increment]]
[[= reflect::unique]]
[[= reflect::indexed]]
```

`reflect::id` is an alias for `reflect::primary_key`.

## Nullability

`std::optional<T>` fields are nullable by default. Non-optional fields are
`NOT NULL` by default.

```cpp
[[= reflect::nullable]]
std::optional<std::string> bio;

[[= reflect::not_null]]
std::string email;
```

## Types

Reflect supports booleans, integral types, floating-point types, enums,
`std::string`, byte vectors, chrono dates/times/timestamps, and
`std::optional<T>` for those field types.

Common type annotations:

```cpp
[[= reflect::varchar{320}]]
[[= reflect::text]]
[[= reflect::json]]
[[= reflect::uuid]]
[[= reflect::blob]]
[[= reflect::decimal{12, 2}]]
[[= reflect::date]]
[[= reflect::time]]
[[= reflect::timestamp]]
```

Use `reflect::sql_type{"..."}` when you need a backend-specific declaration:

```cpp
[[= reflect::sql_type{"CITEXT"}]]
std::string email;
```

## Defaults And Checks

Defaults and checks are raw SQL fragments:

```cpp
[[= reflect::default_value{"'draft'"}]]
std::string status = "draft";

[[= reflect::check{"total >= 0"}]]
double total = 0.0;
```

Reflect does not quote or sanitize these fragments. They are part of the schema,
not user input.

## Created And Updated Timestamps

```cpp
[[= reflect::created_at, = reflect::timestamp]]
std::chrono::system_clock::time_point created_at{};

[[= reflect::updated_at, = reflect::timestamp]]
std::chrono::system_clock::time_point updated_at{};
```

`created_at` and `updated_at` get a `CURRENT_TIMESTAMP` default. Both are skipped
on insert. On update, `updated_at` is set by SQL rather than by the C++ object.

## Foreign Keys

```cpp
[[= reflect::references{"users", "id", "CASCADE"}]]
std::int64_t user_id = 0;
```

The positional fields are:

1. referenced table
2. referenced column
3. `ON DELETE` action
4. `ON UPDATE` action

Use the fourth value only when updates to the referenced key should cascade too.

## Ignored Fields

```cpp
[[= reflect::ignore]]
std::string cached_display_label;
```

Ignored fields are omitted from descriptors, DDL, inserts, updates, selects, and
row materialization.

## Model Rules

Reflect validates model shape at compile time:

- models must be default-initializable
- models must have at least one reflected non-static data member
- at least one persisted field is required
- there must be at most one primary key
- if no primary key annotation exists, a persisted field named `id` becomes the
  implicit primary key
- primary keys cannot be `std::optional<T>`
- `auto_increment` requires a non-optional integral field, excluding `bool`
- temporal annotations require compatible chrono or string storage
