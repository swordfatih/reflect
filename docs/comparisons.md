---
layout: default
title: Comparisons
nav_order: 13
---

# Comparisons

Reflect borrows ideas from mature ORMs, but it is not trying to copy any single
library.

## Prisma

Prisma uses an external schema file and generated clients. Reflect uses C++
models directly as the schema source. Reflect does not yet provide Prisma-style
schema diffing, migration generation, Studio, or a separate query engine.

## Sequelize

Sequelize exposes model definitions, sync, and migration tooling for JavaScript.
Reflect has comparable basic CRUD and sync concepts, but migration generation
and CLI workflows are still missing.

## TypeORM

TypeORM combines decorators, repositories, relations, and migrations. Reflect's
annotations serve a similar metadata role in C++, but Reflect currently has only
simple relation helpers, not rich eager loading or relation query planning.

## Diesel

Diesel emphasizes compile-time safety and explicit migrations. Reflect shares
the type-safety goal, but Diesel has a much more mature migration and query DSL.
Reflect's advantage is deriving table metadata from C++26 reflection instead of
requiring a separate schema module.

## SeaORM

SeaORM provides async Rust APIs, entities, relations, and migration tooling.
Reflect does not yet have async APIs, generated entities, or comparable relation
loading. Reflect focuses on minimal runtime code around reflected C++ models.

## SQLx

SQLx validates SQL and offers migrations without being a full ORM. Reflect is
more model-driven, while SQLx is more SQL-first. Reflect currently lacks SQLx's
level of async runtime integration and compile-time query checking.

## C++ ORMs And SQL Libraries

- ODB: mature object persistence and schema evolution, but requires a dedicated
  compiler/tooling flow.
- sqlpp11: strong typed SQL expression building, but it is SQL DSL-first rather
  than reflection-model-first.
- SOCI: database access abstraction rather than a full model-driven ORM.
- Wt::Dbo: object persistence with relations and sessions; Reflect is lighter
  and centered on C++26 reflection.

## Current Positioning

Reflect is closest to a lightweight C++ model-first ORM:

- more schema-aware than raw database wrappers
- less mature than established migration systems
- more automatic than typed SQL DSLs
- intentionally conservative about destructive schema changes
