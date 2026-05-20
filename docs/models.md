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
