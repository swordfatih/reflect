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

## Design Principles

- The C++ model is the source of truth.
- Runtime SQL always uses bound values for generated predicates.
- Safe migration is conservative; destructive migration is explicit.
- Validation is separate from migration so production startup can fail fast on
  drift.

## Current Status

Reflect is pre-1.0. It is suitable for experiments and early applications, but
the stable V1 line still needs generated migration files, broader backend
coverage, packaging, and more relation query planning.

## Start Here

1. Read [Getting Started](getting-started/).
2. Define models with [Model Annotations](models/).
3. Learn the [Query API](querying/).
4. Use [Schema Validation](schema-validation/) before production writes.
5. Understand [Migrations](migrations/) before changing existing databases.
