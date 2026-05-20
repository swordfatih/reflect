#include <reflect/client.hpp>

#include <stdexcept>
#include <string>
#include <string_view>

namespace reflect
{
namespace
{

[[nodiscard]] bool starts_with(std::string_view value, std::string_view prefix)
{
    return value.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] std::string sqlite_path_from_uri(std::string_view uri)
{
    constexpr std::string_view prefix = "sqlite://";
    std::string path{uri.substr(prefix.size())};

    if(path.empty())
    {
        throw std::invalid_argument{"sqlite URI must include a database path, for example sqlite://app.db or sqlite://:memory:"};
    }

    return path;
}

} // namespace

client::client(std::string_view uri)
{
    if(starts_with(uri, "sqlite://"))
    {
        backend_ = detail::make_sqlite_backend(sqlite_path_from_uri(uri));
        return;
    }

    if(starts_with(uri, "postgres://") || starts_with(uri, "postgresql://"))
    {
        backend_ = detail::make_postgresql_backend(std::string{uri});
        return;
    }

    throw std::invalid_argument{"unsupported reflect ORM URI. Expected sqlite://, postgres://, or postgresql://"};
}

client::~client() = default;

client::client(client&&) noexcept = default;

client& client::operator=(client&&) noexcept = default;

} // namespace reflect

