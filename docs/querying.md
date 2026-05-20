---
layout: default
title: Querying
nav_order: 4
---

# Querying

Reflect builds SQL from model metadata and typed field predicates. Values are
bound as parameters.

## Filters

```cpp
auto active = reflect::where(&User::email).ends_with("@example.test");
auto older = reflect::where(&User::id).gt(100);
auto either = active || older;
```

Supported predicate helpers:

- `eq`, `ne`
- `lt`, `lte`, `gt`, `gte`
- `between`
- `in`, `in_range`
- `is_null`, `is_not_null`
- `starts_with`, `ends_with`, `contains`

`eq(std::nullopt)` emits `IS NULL`; `ne(std::nullopt)` emits `IS NOT NULL`.

## Query Builder

```cpp
auto query =
    reflect::query<User>(reflect::where(&User::email).contains("example"))
        .order_by(&User::name)
        .take(25)
        .skip(50);

auto users = db.find_many<User>(query);
```

## CRUD

```cpp
db.insert<User>(user);
db.insert_many<User>(users);
db.find<User>(id);
db.find_one<User>(reflect::where(&User::email).eq("ada@example.test"));
db.update<User>(user);
db.delete_many<User>(reflect::where(&User::email).contains("test"));
db.count<User>();
db.exists<User>(reflect::where(&User::email).eq("ada@example.test"));
```

## Relations

```cpp
auto posts = db.table<User>().has_many<Post>(user, &Post::user_id);
auto author = db.table<Post>().belongs_to<User>(post, &Post::user_id);
```

These helpers run separate queries. Join planning and eager loading are planned
for a later V1 milestone.
