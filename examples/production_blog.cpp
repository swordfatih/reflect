#include <reflect/reflect.hpp>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

enum class PostStatus : std::int32_t
{
    draft = 0,
    published = 1,
    archived = 2,
};

struct [[= reflect::table{"accounts"}]] Account
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::unique, = reflect::not_null, = reflect::varchar{120}]]
    std::string name;

    [[= reflect::not_null, = reflect::varchar{40}, = reflect::default_value{"'free'"}]]
    std::string plan = "free";

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};

    [[= reflect::updated_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point updated_at{};
};

struct [[= reflect::table{"users"}]] User
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"accounts", "id", "CASCADE", "CASCADE"}]]
    std::int64_t account_id = 0;

    [[= reflect::unique, = reflect::indexed, = reflect::not_null, = reflect::varchar{320}]]
    std::string email;

    [[= reflect::not_null, = reflect::varchar{120}]]
    std::string display_name;

    [[= reflect::nullable, = reflect::text]]
    std::optional<std::string> bio;

    [[= reflect::json, = reflect::not_null, = reflect::default_value{"'{}'"}]]
    std::string settings_json = "{}";

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};

    [[= reflect::updated_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point updated_at{};
};

struct [[= reflect::table{"posts"}]] Post
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"users", "id", "CASCADE", "CASCADE"}]]
    std::int64_t author_id = 0;

    [[= reflect::not_null, = reflect::varchar{200}]]
    std::string title;

    [[= reflect::unique, = reflect::indexed, = reflect::not_null, = reflect::varchar{220}]]
    std::string slug;

    [[= reflect::nullable, = reflect::varchar{280}]]
    std::optional<std::string> summary;

    [[= reflect::text, = reflect::not_null]]
    std::string body;

    [[= reflect::indexed, = reflect::not_null, = reflect::default_value{"0"}]]
    PostStatus status = PostStatus::draft;

    [[= reflect::nullable, = reflect::date]]
    std::optional<std::chrono::sys_days> published_on;

    [[= reflect::json, = reflect::not_null, = reflect::default_value{"'{}'"}]]
    std::string metadata_json = "{}";

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};

    [[= reflect::updated_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point updated_at{};
};

struct [[= reflect::table{"comments"}]] Comment
{
    [[= reflect::id, = reflect::auto_increment]]
    std::int64_t id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"posts", "id", "CASCADE", "CASCADE"}]]
    std::int64_t post_id = 0;

    [[= reflect::indexed, = reflect::not_null, = reflect::references{"users", "id", "CASCADE", "CASCADE"}]]
    std::int64_t author_id = 0;

    [[= reflect::text, = reflect::not_null]]
    std::string body;

    [[= reflect::indexed, = reflect::not_null, = reflect::default_value{"0"}]]
    bool approved = false;

    [[= reflect::created_at, = reflect::timestamp]]
    std::chrono::system_clock::time_point created_at{};
};

int main()
{
    reflect::client db{"sqlite://:memory:"};

    db.transaction([](reflect::client& tx) {
        tx.migrate<Account>();
        tx.migrate<User>();
        tx.migrate<Post>();
        tx.migrate<Comment>();
    });

    db.apply_migrations({
        reflect::migration{
            .id = "001_published_post_summaries",
            .statements = {
                reflect::statement{
                    .sql =
                        "CREATE VIEW IF NOT EXISTS \"published_post_summaries\" AS "
                        "SELECT \"posts\".\"id\", \"posts\".\"title\", \"users\".\"email\" AS \"author_email\" "
                        "FROM \"posts\" JOIN \"users\" ON \"users\".\"id\" = \"posts\".\"author_id\" "
                        "WHERE \"posts\".\"status\" = 1",
                },
            },
        },
    });

    auto accounts = db.table<Account>();
    auto users = db.table<User>();
    auto posts = db.table<Post>();
    auto comments = db.table<Comment>();

    const auto account = accounts.insert(Account{.name = "Reflect Labs", .plan = "team"});
    const auto ada = users.insert(User{
        .account_id = account.last_insert_id,
        .email = "ada@reflect.test",
        .display_name = "Ada",
        .bio = "Computing notes and database design.",
        .settings_json = R"({"theme":"light","digest":true})",
    });

    const auto post = posts.insert(Post{
        .author_id = ada.last_insert_id,
        .title = "Designing a reflection-first ORM",
        .slug = "reflection-first-orm",
        .summary = "A production schema with typed filters, relations, JSON, dates, and migrations.",
        .body = "Reflect keeps C++ models as the source of truth while still emitting explicit SQL.",
        .status = PostStatus::published,
        .published_on = std::chrono::sys_days{std::chrono::year{2026} / 5 / 20},
        .metadata_json = R"({"tags":["orm","cpp","reflection"],"readingMinutes":6})",
    });

    comments.insert_many({
        Comment{.post_id = post.last_insert_id, .author_id = ada.last_insert_id, .body = "First approved comment.", .approved = true},
        Comment{.post_id = post.last_insert_id, .author_id = ada.last_insert_id, .body = "Needs moderation.", .approved = false},
    });

    const auto published_posts =
        posts.find_many(
            reflect::query<Post>(
                reflect::where(&Post::status).eq(PostStatus::published) &&
                reflect::where(&Post::summary).is_not_null()
            )
                .order_by_desc(&Post::created_at)
                .take(10)
        );

    for(const auto& published_post: published_posts)
    {
        const auto author = posts.belongs_to<User>(published_post, &Post::author_id);
        const auto approved_comments = comments.count(
            reflect::where(&Comment::post_id).eq(published_post.id) &&
            reflect::where(&Comment::approved).eq(true)
        );

        spdlog::info(
            "{} by {} ({} approved comments)",
            published_post.title,
            author ? author->email : "unknown",
            approved_comments
        );
    }

    if(auto existing = users.find(ada.last_insert_id))
    {
        existing->display_name = "Ada Lovelace";
        users.update(*existing);
    }

    if(auto saved_account = accounts.find(account.last_insert_id))
    {
        const auto account_users = accounts.has_many<User>(*saved_account, &User::account_id);
        spdlog::info("account has {} user(s)", account_users.size());
    }
}
