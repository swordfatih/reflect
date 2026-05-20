#include <reflect/reflect.hpp>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include <spdlog/spdlog.h>

struct [[= reflect::table{"customers"}]] Customer
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::unique, = reflect::indexed, = reflect::not_null, = reflect::varchar{320}]]
    std::string email;

    [[= reflect::not_null, = reflect::varchar{160}]]
    std::string display_name;

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};

    [[= reflect::updated_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point updated_at{};
};

struct [[= reflect::table{"invoices"}]] Invoice
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"customers", "id", "CASCADE"}]]
    std::int64_t customer_id = 0;

    [[= reflect::not_null, = reflect::decimal{12, 2}]]
    double total = 0.0;

    [[= reflect::not_null, = reflect::varchar{40}, = reflect::default_value{"'open'"}]]
    std::string status = "open";

    [[= reflect::nullable, = reflect::date]]
    std::optional<std::chrono::sys_days> due_on;

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};
};

void create_legacy_table_if_missing(reflect::detail::backend& sqlite)
{
    if(sqlite.inspect_table("customers").exists())
    {
        return;
    }

    sqlite.execute(reflect::statement{
        .sql =
            "CREATE TABLE \"customers\" ("
            "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
            "\"email\" TEXT, "
            "\"full_name\" TEXT"
            ")",
    });

    sqlite.execute(reflect::statement{
        .sql = "INSERT INTO \"customers\" (\"email\", \"full_name\") VALUES (?, ?)",
        .binds = {std::string{"legacy@example.test"}, std::string{"Legacy Customer"}},
    });
}

int main()
{
    auto sqlite = reflect::detail::make_sqlite_backend(":memory:");
    create_legacy_table_if_missing(*sqlite);

    reflect::table_client<Customer> customers{*sqlite};
    reflect::table_client<Invoice> invoices{*sqlite};

    const auto drift = customers.validate_schema();
    if(!drift.valid())
    {
        spdlog::warn("legacy customers table has {} schema issue(s)", drift.issues.size());
    }

    try
    {
        customers.migrate(reflect::schema_sync_options{
            .validate_after = false,
        });
        customers.require_schema();
    }
    catch(const reflect::orm_error& error)
    {
        spdlog::warn("additive migration could not fix destructive drift: {}", error.what());
    }

    customers.migrate_force();
    invoices.migrate();

    const auto customer = customers.insert(Customer{
        .email = "billing@example.test",
        .display_name = "Billing Team",
    });

    invoices.insert(Invoice{
        .customer_id = customer.last_insert_id,
        .total = 199.95,
        .status = "open",
        .due_on = std::chrono::sys_days{std::chrono::year{2026} / 6 / 15},
    });

    reflect::apply_migration(*sqlite, reflect::migration{
        .id = "002_invoice_open_index",
        .statements = {
            reflect::statement{
                .sql =
                    "CREATE INDEX IF NOT EXISTS \"idx_invoices_open\" "
                    "ON \"invoices\" (\"status\") WHERE \"status\" = 'open'",
            },
        },
    });

    spdlog::info("open invoices: {}", invoices.count(reflect::where(&Invoice::status).eq("open")));
}
