---
layout: home
title: Home
nav_order: 1
---

# reflect

`reflect` is a C++26 reflection-first ORM. Your C++ aggregate models define the
database contract, and Reflect derives schema metadata, DDL, queries, CRUD,
validation, and migrations from those models.

```cpp
struct [[= reflect::table{"users"}]] User
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::unique, = reflect::indexed, = reflect::not_null, = reflect::varchar{320}]]
    std::string email;

    [[= reflect::not_null, = reflect::varchar{120}]]
    std::string name;
};
```

## What It Gives You

- SQLite and PostgreSQL backends.
- Reflection-based schema descriptors.
- Type-safe filters and bound SQL parameters.
- CRUD, counts, existence checks, transactions, and relation helpers.
- Additive migrations and versioned manual migrations.
- Schema validation and drift reporting.
- Destructive development reset when you explicitly ask for it.

## Current Status

Reflect is pre-1.0. It is suitable for experiments and early applications, but
the stable V1 line still needs generated migration files, broader backend
coverage, packaging, and more relation query planning.

## Start Here

1. Read [Getting Started](getting-started/).
2. Define models with [Model Annotations](models/).
3. Use [Schema Validation](schema-validation/) before production writes.
4. Understand [Migrations](migrations/) before changing existing databases.
