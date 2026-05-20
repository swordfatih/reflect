---
layout: default
title: Roadmap
nav_order: 12
---

# Roadmap

Reflect's complete V1 should be judged against mature ORMs without copying any
single one directly.

## Migration System

- generate migration files from model diffs
- support up/down migrations
- detect whole-database drift
- plan SQLite table rebuilds for destructive changes
- represent rename/drop/type-change operations explicitly
- add a CLI for migration generation and application

## Query System

- joins
- eager loading
- nested relation includes
- many-to-many helpers
- relation-aware filters
- partial update objects
- aggregate queries and grouping

## Runtime

- connection pooling
- async APIs
- statement caching
- retry policies
- query timeouts
- structured query logging
- tracing and metrics hooks

## Backends

- stronger PostgreSQL type handling
- MySQL/MariaDB backend
- SQL Server backend
- backend-specific capabilities matrix

## Distribution

- install targets
- package manager recipes
- compatibility matrix for compilers and database versions
- generated API reference
