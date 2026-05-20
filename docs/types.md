---
layout: default
title: Types
nav_order: 4
---

# Types

Reflect supports a deliberately small value model. Generated statements bind
values as `reflect::sql_value`:

```cpp
using bytes = std::vector<std::byte>;

using sql_value = std::variant<
    std::monostate,
    bool,
    std::int64_t,
    std::uint64_t,
    double,
    std::string,
    bytes
>;
```

`std::monostate` represents SQL `NULL`.

## Supported Model Field Types

- `bool`
- signed and unsigned integral types
- floating-point types
- enums
- `std::string`
- `std::vector<std::byte>`
- `std::chrono::sys_days`
- `std::chrono::system_clock::time_point`
- chrono durations
- `std::optional<T>` for supported scalar types

Unsupported fields should be marked with `reflect::ignore`.

## Default SQL Type Mapping

| C++ field | SQLite | PostgreSQL |
| --- | --- | --- |
| `bool` | `INTEGER` | `BOOLEAN` |
| 32-bit-or-smaller integral | `INTEGER` | `INTEGER` |
| larger integral | `INTEGER` | `BIGINT` |
| floating point | `REAL` | `DOUBLE PRECISION` |
| `std::string` | `TEXT` | `TEXT` |
| `std::chrono::sys_days` | `TEXT` | `DATE` |
| `std::chrono::system_clock::time_point` | `TEXT` | `TIMESTAMP` |
| chrono duration | `TEXT` | `TIME` |
| `std::vector<std::byte>` | `BLOB` | `BYTEA` |

Annotations can override this mapping:

- `varchar`
- `decimal`
- `text`
- `blob`
- `json`
- `uuid`
- `date`
- `time`
- `timestamp`
- `sql_type`

## Date And Time Formatting

SQLite stores date/time values as text:

- date: `YYYY-MM-DD`
- time: `HH:MM:SS`
- timestamp: `YYYY-MM-DD HH:MM:SS`

Materialization expects the same formats. Invalid dates, out-of-day time values,
and `NULL` into non-optional temporal fields throw.

## Numeric Safety

Reflect checks integer overflow when reading database values into narrower C++
integral fields. A value that does not fit the destination field throws instead
of silently truncating.

## BLOB Values

Use:

```cpp
reflect::bytes payload;
```

or:

```cpp
std::vector<std::byte> payload;
```

Non-BLOB values cannot be materialized into byte vectors.
