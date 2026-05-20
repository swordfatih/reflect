---
layout: default
title: Statements
nav_order: 10
---

# Statements

Reflect exposes generated SQL as `statement` objects:

```cpp
struct statement
{
    std::string sql;
    std::vector<sql_value> binds;
};
```

Generated SQL uses `?` placeholders. Backends translate or bind them as needed.

## Schema Statements

```cpp
auto model = reflect::describe_model<User>(reflect::dialect::sqlite);

auto create = reflect::create_table(model);
auto drop = reflect::drop_table(model);
auto indexes = reflect::create_index_statements(model);
auto schema = reflect::create_schema_statements(model);
```

`create_schema_statements` returns `CREATE TABLE IF NOT EXISTS` followed by
index statements.

## CRUD Statements

```cpp
auto insert = reflect::insert_statement(user, model);
auto upsert = reflect::upsert_statement(user, model);
auto update = reflect::update_statement(user, model);
auto select = reflect::select_statement(model);
auto one = reflect::select_one_statement(model, filter);
auto by_id = reflect::select_by_id_statement(model, id);
auto del = reflect::delete_statement(model, filter);
auto count = reflect::count_statement(model, filter);
auto exists = reflect::exists_statement(model, filter);
```

These functions are useful for tests, tooling, logging, and building custom
execution flows.

## Identifier Quoting

Reflect quotes generated identifiers with double quotes:

```sql
"users"
"email"
```

Embedded double quotes in identifiers are escaped.

## Raw SQL

Raw statements can be executed directly:

```cpp
db.execute(reflect::statement{
    .sql = "CREATE VIEW \"active_users\" AS SELECT * FROM \"users\"",
});
```

For values, use binds:

```cpp
db.query(reflect::statement{
    .sql = "SELECT \"email\" FROM \"users\" WHERE \"id\" = ?",
    .binds = {std::int64_t{42}},
});
```

PostgreSQL raw statements can use native `$1`, `$2`, ... placeholders. When a
PostgreSQL statement contains native placeholders, Reflect does not rewrite `?`
characters, so backend-specific operators such as JSONB `?` can be used.

Do not concatenate user input into raw SQL.
