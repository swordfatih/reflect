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
cmake -S . -B build -DREFLECT_BUILD_TESTS=ON -DREFLECT_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests and examples are off by default so Reflect can be consumed by another
CMake project without building development targets.

The SQLite backend is enabled by default and uses bundled SQLite through libsl3.
The PostgreSQL backend is disabled by default, so the default build does not
require PostgreSQL or libpq to be installed.

## Consume With FetchContent

```cmake
cmake_minimum_required(VERSION 3.30)

project(my_app LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    reflect
    GIT_REPOSITORY https://github.com/swordfatih/reflect.git
    GIT_TAG main
)

set(REFLECT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(REFLECT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(reflect)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE reflect::reflect)
```

To enable the PostgreSQL backend, set the option before
`FetchContent_MakeAvailable(reflect)`:

```cmake
set(REFLECT_ENABLE_POSTGRESQL ON CACHE BOOL "" FORCE)
```

PostgreSQL support uses taocpp/taopq and requires the PostgreSQL client library
and headers (`libpq`) to be installed where CMake can find them.

Reflect requires C++26 static reflection support. The `reflect::reflect` target
adds the required `-freflection` compile option to consumers.

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
the connection string to the PostgreSQL backend when Reflect is built with
`REFLECT_ENABLE_POSTGRESQL=ON`.

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
