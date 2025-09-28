#pragma once

namespace Lab {

class App;

namespace internal {
inline auto get_app() -> App*&
{
    static App* instance{};
    return instance;
}
} // namespace internal

} // namespace Lab