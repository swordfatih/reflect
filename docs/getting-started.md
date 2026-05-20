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
