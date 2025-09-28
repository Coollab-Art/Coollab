#if !DOCTEST_CONFIG_DISABLE
#define DOCTEST_CONFIG_IMPLEMENT
#include <Cool/Path/Path.h>
#include <doctest/doctest.h>

class PathsConfig : public Cool::PathsConfig {
};

// NB: the tests are defined alongside the file that they test, not here.
auto main() -> int
{
    Cool::Path::initialize<PathsConfig>();
    return doctest::Context{}.run();
}

#else
auto main() -> int {}
#endif