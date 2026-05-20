---
layout: default
title: Getting Started
nav_order: 2
---

# Getting Started

Reflect requires a compiler with C++26 static reflection support and the
`-freflection` compiler flag.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Connect

```cpp
reflect::client sqlite{"sqlite://:memory:"};
reflect::client file_db{"sqlite://app.db"};
reflect::client postgres{"postgresql://user:pass@localhost/app"};
```

Supported URI prefixes are:

- `sqlite://`
- `postgres://`
- `postgresql://`

SQLite accepts `:memory:` and file paths after the prefix. PostgreSQL forwards
the connection string to the PostgreSQL backend.

## Create Or Sync A Table

```cpp
db.migrate<User>();
```

`migrate<T>()` creates the table if missing, adds missing columns, creates
indexes, and then validates the schema.

## Validate On Startup

```cpp
db.require_schema<User>();
db.require_schema<Post>();
```

Use validation as a startup guard before the application starts accepting
writes.

## Query

```cpp
auto users = db.find_many<User>(
    reflect::query<User>(reflect::where(&User::email).contains("@example.test"))
        .order_by(&User::name)
        .take(50)
);
```

## Transactions

```cpp
db.transaction([](reflect::client& tx) {
    tx.insert<User>({.email = "ada@example.test", .name = "Ada"});
    tx.insert<User>({.email = "grace@example.test", .name = "Grace"});
});
```

Nested transactions use savepoints.

## Inspect A Table

```cpp
auto table = db.inspect<User>();

if(table.exists())
{
    for(const auto& column: table.columns)
    {
        // column.name, column.sql_type, column.nullable, column.primary_key
    }
}
```

Use `inspect_table("table_name")` when you do not have a model type.

## Direct SQL

```cpp
db.execute(reflect::statement{
    .sql = "CREATE INDEX IF NOT EXISTS \"idx_users_email\" ON \"users\" (\"email\")",
});

auto rows = db.query(reflect::statement{
    .sql = "SELECT \"email\" FROM \"users\" WHERE \"id\" = ?",
    .binds = {std::int64_t{1}},
});
```

Use direct SQL for backend-specific features, views, custom indexes, and manual
migrations.
