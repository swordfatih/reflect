---
layout: default
title: Errors
nav_order: 9
---

# Errors

Reflect uses exception types under `reflect::orm_error`.

```cpp
try
{
    db.insert<User>(user);
}
catch(const reflect::unique_violation& error)
{
    // duplicate key
}
catch(const reflect::orm_error& error)
{
    // other Reflect error
}
```

## Error Hierarchy

```text
orm_error
├── connection_error
├── transaction_error
├── migration_error
├── schema_validation_error
└── sql_error
    ├── query_error
    └── constraint_error
        ├── unique_violation
        ├── foreign_key_violation
        ├── not_null_violation
        └── check_violation
```

`schema_validation_error` is declared in `schema_validation.hpp`.

## SQL Errors

`sql_error` includes the SQL statement that failed:

```cpp
catch(const reflect::sql_error& error)
{
    auto sql = error.sql();
}
```

Backends classify common constraint messages into more specific error classes.
Classification is based on backend error text, so unknown database errors fall
back to `query_error`.

## Migration Errors

`migration_error` wraps failures while applying versioned migrations. If a
transactional migration fails after `BEGIN`, Reflect attempts `ROLLBACK` before
throwing.

## Transaction Errors

`transaction_error` is used when rollback itself fails. Normal exceptions thrown
inside a transaction body are rethrown after rollback.

## Validation Errors

`schema_validation_error` stores structured drift details:

```cpp
catch(const reflect::schema_validation_error& error)
{
    const auto& result = error.result();
}
```
