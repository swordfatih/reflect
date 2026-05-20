---
layout: default
title: API Reference
nav_order: 11
---

# API Reference

This page summarizes the public API surface. Namespaces are omitted where they
are clearly `reflect::`.

## Client

```cpp
client(std::string_view uri);

[[nodiscard]] dialect target_dialect() const noexcept;
[[nodiscard]] table_info inspect_table(std::string_view table);
template <typename Model> [[nodiscard]] table_info inspect();

template <typename Function> decltype(auto) transaction(Function&& function);
template <typename Model> [[nodiscard]] table_client<Model> table();

template <typename Model> void migrate();
template <typename Model> void migrate(schema_sync_options options);
template <typename Model> void migrate_force(schema_validation_options validation = {});
template <typename Model> void reset_schema();
template <typename Model> void migrate_versioned(std::string_view id);
template <typename Model> void migrate_versioned();
void apply_migrations(const std::vector<migration>& migrations);

template <typename Model> [[nodiscard]] schema_validation_result validate_schema(schema_validation_options options = {});
template <typename Model> void require_schema(schema_validation_options options = {});

template <typename Model> [[nodiscard]] insert_result insert(const Model& model);
template <typename Model> [[nodiscard]] insert_result create(const Model& model);
template <typename Model> [[nodiscard]] insert_result insert_many(const std::vector<Model>& models);
template <typename Model> [[nodiscard]] insert_result create_many(const std::vector<Model>& models);
template <typename Model> [[nodiscard]] insert_result upsert(const Model& model);

template <typename Model, typename Id> [[nodiscard]] std::optional<Model> find(Id&& id);
template <typename Model, typename Id> [[nodiscard]] std::optional<Model> find_unique(Id&& id);
template <typename Model> [[nodiscard]] std::optional<Model> find_one(condition filter = {});
template <typename Model> [[nodiscard]] std::optional<Model> find_one(query_options<Model> options);
template <typename Model> [[nodiscard]] std::optional<Model> find_one(const query_builder<Model>& query);
template <typename Model> [[nodiscard]] std::vector<Model> find_many(condition filter = {});
template <typename Model> [[nodiscard]] std::vector<Model> find_many(query_options<Model> options);
template <typename Model> [[nodiscard]] std::vector<Model> find_many(const query_builder<Model>& query);

template <typename Model> [[nodiscard]] std::uint64_t update(const Model& model);
template <typename Model> [[nodiscard]] std::uint64_t update_many(const Model& patch, condition filter);
template <typename Model> [[nodiscard]] std::uint64_t update_many(const Model& patch, query_options<Model> options);

template <typename Model> [[nodiscard]] std::uint64_t remove(condition filter);
template <typename Model> [[nodiscard]] std::uint64_t delete_many(condition filter);
template <typename Model> [[nodiscard]] std::uint64_t delete_many(query_options<Model> options);
template <typename Model> [[nodiscard]] std::uint64_t delete_all();
template <typename Model, typename Id> [[nodiscard]] std::uint64_t remove_by_id(Id&& id);

template <typename Model> [[nodiscard]] std::uint64_t count(condition filter = {});
template <typename Model> [[nodiscard]] std::uint64_t count(query_options<Model> options);
template <typename Model> [[nodiscard]] bool exists(condition filter = {});
template <typename Model> [[nodiscard]] bool exists(query_options<Model> options);

[[nodiscard]] detail::execution_result execute(const statement& input);
[[nodiscard]] std::vector<detail::row> query(const statement& input);
```

## Table Client

`table_client<Model>` exposes the same model-specific operations and relation
helpers:

```cpp
has_many<Related>(parent, &Related::foreign_key);
has_one<Related>(parent, &Related::foreign_key);
belongs_to<Parent>(child, &Model::foreign_key);
```

`table_client<Model>::descriptor()` returns the reflected `model_descriptor`.

## Schema Sync Options

```cpp
struct schema_sync_options
{
    bool validate_after = true;
    bool force = false;
    schema_validation_options validation{};
};
```

`force = true` is destructive when drift is detected.

## Schema Validation Options

```cpp
struct schema_validation_options
{
    bool allow_extra_columns = false;
    bool check_types = true;
    bool strict_sql_types = false;
    bool check_nullability = true;
    bool check_primary_key = true;
    bool check_defaults = true;
    bool check_indexes = true;
    bool check_foreign_keys = true;
};
```

## Statements

```cpp
struct statement
{
    std::string sql;
    std::vector<sql_value> binds;
};
```

Use raw statements for hand-written migrations or backend-specific SQL.

`upsert` uses primary-key conflict handling on backends that support
`ON CONFLICT`.

## Introspection

```cpp
struct table_info
{
    std::string name;
    std::vector<column_info> columns;
    std::vector<index_info> indexes;
    std::vector<foreign_key_info> foreign_keys;
    bool exists() const noexcept;
};
```

## Insert Result

```cpp
struct insert_result
{
    std::uint64_t rows_affected = 0;
    std::int64_t last_insert_id = 0;
};
```

`last_insert_id` is SQLite-oriented. PostgreSQL users should use explicit
backend-specific returning SQL when needed.

## Query Options Caveat

`query_options<Model>` supports filters, orderings, limit, and offset for
selects. `count`, `exists`, `update_many`, and `delete_many` currently consume
only the filter portion.
