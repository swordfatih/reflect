---
layout: default
title: Backends
nav_order: 8
---

# Backends

Reflect currently ships SQLite and PostgreSQL backends. SQLite is enabled by
default. PostgreSQL is opt-in with `REFLECT_ENABLE_POSTGRESQL=ON`.

## Client URIs

```cpp
reflect::client sqlite_memory{"sqlite://:memory:"};
reflect::client sqlite_file{"sqlite://app.db"};
reflect::client postgres{"postgresql://user:pass@localhost/app"};
```

Accepted prefixes:

- `sqlite://`
- `postgres://`
- `postgresql://`

Unsupported prefixes throw `std::invalid_argument`.

## SQLite

The SQLite backend:

- binds values through the SQLite wrapper
- maps `std::uint64_t` only when it fits SQLite's signed integer range
- enables `PRAGMA foreign_keys = ON` per connection
- sets `PRAGMA busy_timeout = 5000`
- introspects columns with `PRAGMA table_info`
- introspects indexes with `PRAGMA index_list` and `PRAGMA index_info`
- introspects foreign keys with `PRAGMA foreign_key_list`

SQLite reports many string-like model types as `TEXT`; schema validation uses
type-family matching by default to avoid false drift.

## PostgreSQL

The PostgreSQL backend is disabled by default. Enable it with
`REFLECT_ENABLE_POSTGRESQL=ON`; it requires the PostgreSQL client library and
headers (`libpq`) for taocpp/taopq.

The PostgreSQL backend:

- converts `?` placeholders into `$1`, `$2`, ... outside quoted SQL text and
  comments
- leaves SQL with native `$1`, `$2`, ... placeholders unchanged, which is useful
  for raw PostgreSQL statements that use operators containing `?`
- supports up to 1024 bound parameters per statement
- sends scalar parameters as text
- sends byte vectors as PostgreSQL hex bytea text
- introspects columns through `information_schema.columns`
- introspects indexes through `pg_class`, `pg_index`, and `pg_attribute`
- introspects foreign keys through `information_schema`

PostgreSQL `INSERT` currently reports rows affected but not `last_insert_id`.
Use explicit returning SQL through `query(statement)` if you need backend-specific
returning behavior.

## Direct Backend Use

Advanced examples can construct a backend directly:

```cpp
auto sqlite = reflect::detail::make_sqlite_backend(":memory:");
reflect::table_client<User> users{*sqlite};
```

The direct backend API is lower level. Prefer `reflect::client` unless you are
building tooling or examples that intentionally bypass URI parsing.
