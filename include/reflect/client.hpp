#pragma once

#include <reflect/detail/backend.hpp>
#include <reflect/error.hpp>
#include <reflect/introspection.hpp>
#include <reflect/query.hpp>
#include <reflect/statement.hpp>
#include <reflect/table.hpp>

#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace reflect
{

class client
{
public:
    explicit client(std::string_view uri);
    ~client();

    client(client&&) noexcept;
    client& operator=(client&&) noexcept;

    client(const client&) = delete;
    client& operator=(const client&) = delete;

    [[nodiscard]] dialect target_dialect() const noexcept
    {
        return backend_->target_dialect();
    }

    [[nodiscard]] table_info inspect_table(std::string_view table)
    {
        return backend_->inspect_table(table);
    }

    template <typename Model>
    [[nodiscard]] table_info inspect()
    {
        return backend_->inspect_table(table_name<Model>());
    }

    template <typename Function>
    decltype(auto) transaction(Function&& function)
    {
        const auto level = transaction_depth_;
        const auto savepoint = "reflect_tx_" + std::to_string(level);
        const bool nested = level != 0;

        begin_transaction(nested, savepoint);
        ++transaction_depth_;
        bool active = true;

        try
        {
            using result_type = std::conditional_t<
                std::is_invocable_v<Function&, client&>,
                std::invoke_result<Function&, client&>,
                std::invoke_result<Function&>
            >::type;

            if constexpr(std::is_void_v<result_type>)
            {
                if constexpr(std::is_invocable_v<Function&, client&>)
                {
                    std::invoke(function, *this);
                }
                else
                {
                    std::invoke(function);
                }

                --transaction_depth_;
                active = false;
                commit_transaction(nested, savepoint);
            }
            else
            {
                result_type result = [&]() -> result_type {
                    if constexpr(std::is_invocable_v<Function&, client&>)
                    {
                        return std::invoke(function, *this);
                    }
                    else
                    {
                        return std::invoke(function);
                    }
                }();
                --transaction_depth_;
                active = false;
                commit_transaction(nested, savepoint);
                return result;
            }
        }
        catch(...)
        {
            if(active)
            {
                --transaction_depth_;
                rollback_transaction(nested, savepoint);
            }

            throw;
        }
    }

    template <typename Model>
    [[nodiscard]] table_client<Model> table()
    {
        return table_client<Model>{*backend_};
    }

    template <typename Model>
    void migrate()
    {
        this->template table<Model>().migrate();
    }

    template <typename Model>
    void migrate_versioned(std::string_view id)
    {
        this->template table<Model>().migrate_versioned(id);
    }

    template <typename Model>
    void migrate_versioned()
    {
        this->template table<Model>().migrate_versioned();
    }

    void apply_migrations(const std::vector<migration>& migrations)
    {
        reflect::apply_migrations(*backend_, migrations);
    }

    template <typename Model>
    void sync_schema()
    {
        this->template table<Model>().sync_schema();
    }

    template <typename Model>
    [[nodiscard]] insert_result insert(const Model& model)
    {
        return this->template table<Model>().insert(model);
    }

    template <typename Model>
    [[nodiscard]] insert_result create(const Model& model)
    {
        return this->template table<Model>().create(model);
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t update(const Model& model)
    {
        return this->template table<Model>().update(model);
    }

    template <typename Model>
    [[nodiscard]] std::vector<Model> find_many(condition filter = {})
    {
        return this->template table<Model>().find_many(std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] std::vector<Model> find_many(query_options<Model> options)
    {
        return this->template table<Model>().find_many(std::move(options));
    }

    template <typename Model>
    [[nodiscard]] std::vector<Model> find_many(const query_builder<Model>& query)
    {
        return this->template table<Model>().find_many(query);
    }

    template <typename Model>
    [[nodiscard]] std::optional<Model> find_one(condition filter = {})
    {
        return this->template table<Model>().find_one(std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] std::optional<Model> find_one(query_options<Model> options)
    {
        return this->template table<Model>().find_one(std::move(options));
    }

    template <typename Model>
    [[nodiscard]] std::optional<Model> find_one(const query_builder<Model>& query)
    {
        return this->template table<Model>().find_one(query);
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t count(condition filter = {})
    {
        return this->template table<Model>().count(std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t count(query_options<Model> options)
    {
        return this->template table<Model>().count(std::move(options));
    }

    template <typename Model>
    [[nodiscard]] bool exists(condition filter = {})
    {
        return this->template table<Model>().exists(std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] bool exists(query_options<Model> options)
    {
        return this->template table<Model>().exists(std::move(options));
    }

    template <typename Model>
    [[nodiscard]] insert_result insert_many(const std::vector<Model>& models)
    {
        return this->template table<Model>().insert_many(models);
    }

    template <typename Model>
    [[nodiscard]] insert_result create_many(const std::vector<Model>& models)
    {
        return this->template table<Model>().create_many(models);
    }

    template <typename Model>
    [[nodiscard]] insert_result upsert(const Model& model)
    {
        return this->template table<Model>().upsert(model);
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t update_many(const Model& patch, condition filter)
    {
        return this->template table<Model>().update_many(patch, std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t update_many(const Model& patch, query_options<Model> options)
    {
        return this->template table<Model>().update_many(patch, std::move(options));
    }

    template <typename Model, typename Id>
    [[nodiscard]] std::optional<Model> find(Id&& id)
    {
        return this->template table<Model>().find(std::forward<Id>(id));
    }

    template <typename Model, typename Id>
    [[nodiscard]] std::optional<Model> find_unique(Id&& id)
    {
        return this->template table<Model>().find_unique(std::forward<Id>(id));
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t remove(condition filter)
    {
        return this->template table<Model>().remove(std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t delete_many(condition filter)
    {
        return this->template table<Model>().delete_many(std::move(filter));
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t delete_many(query_options<Model> options)
    {
        return this->template table<Model>().delete_many(std::move(options));
    }

    template <typename Model>
    [[nodiscard]] std::uint64_t delete_all()
    {
        return this->template table<Model>().delete_all();
    }

    template <typename Model, typename Id>
    [[nodiscard]] std::uint64_t remove_by_id(Id&& id)
    {
        return this->template table<Model>().remove_by_id(std::forward<Id>(id));
    }

    [[nodiscard]] detail::execution_result execute(const statement& input)
    {
        return backend_->execute(input);
    }

    [[nodiscard]] std::vector<detail::row> query(const statement& input)
    {
        return backend_->query(input);
    }

private:
    void begin_transaction(bool nested, std::string_view savepoint)
    {
        statement command;
        command.sql = nested ? "SAVEPOINT " + detail::quote_identifier(savepoint) : "BEGIN";
        backend_->execute(command);
    }

    void commit_transaction(bool nested, std::string_view savepoint)
    {
        statement command;
        command.sql = nested ? "RELEASE SAVEPOINT " + detail::quote_identifier(savepoint) : "COMMIT";
        backend_->execute(command);
    }

    void rollback_transaction(bool nested, std::string_view savepoint)
    {
        try
        {
            statement command;
            command.sql = nested ? "ROLLBACK TO SAVEPOINT " + detail::quote_identifier(savepoint) : "ROLLBACK";
            backend_->execute(command);

            if(nested)
            {
                command.sql = "RELEASE SAVEPOINT " + detail::quote_identifier(savepoint);
                backend_->execute(command);
            }
        }
        catch(const std::exception& error)
        {
            throw transaction_error{"reflect transaction rollback failed: " + std::string{error.what()}};
        }
    }

    std::unique_ptr<detail::backend> backend_;
    std::size_t                      transaction_depth_ = 0;
};

} // namespace reflect
