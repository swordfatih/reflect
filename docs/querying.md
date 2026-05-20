---
layout: default
title: Querying
nav_order: 5
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
db.create<User>(user);
db.insert_many<User>(users);
db.create_many<User>(users);
db.find<User>(id);
db.find_unique<User>(id);
db.find_one<User>(reflect::where(&User::email).eq("ada@example.test"));
db.update<User>(user);
db.update_many<User>(patch, reflect::where(&User::email).contains("test"));
db.delete_many<User>(reflect::where(&User::email).contains("test"));
db.count<User>();
db.exists<User>(reflect::where(&User::email).eq("ada@example.test"));
```

`create` is an alias for `insert`; `create_many` is an alias for `insert_many`;
`find_unique` is an alias for primary-key `find`.

## Inserts And Generated Columns

Columns marked as generated on insert are omitted from generated `INSERT`
statements:

- `auto_increment`
- `created_at`
- `updated_at`

When a model has only generated insert columns, Reflect emits `DEFAULT VALUES`.

## Updates

`update(model)` updates by primary key and writes every non-primary-key mutable
column. If a field is marked `updated_at`, Reflect writes the SQL current-time
expression instead of binding the C++ field value.

`update_many(patch, filter)` currently uses a full model instance as the patch.
It is not a sparse update object yet, so default-initialized patch fields can be
written. Use it carefully.

## Deletes

`remove(filter)` and `delete_many(filter)` require a non-empty filter. Use
`delete_all()` when you intentionally want a full-table delete.

`delete_many(query_options)` currently uses only `options.filter`; ordering,
limit, and offset are ignored for deletes.

## Relations

```cpp
auto posts = db.table<User>().has_many<Post>(user, &Post::user_id);
auto author = db.table<Post>().belongs_to<User>(post, &Post::user_id);
```

These helpers run separate queries. Join planning and eager loading are planned
for a later V1 milestone.
