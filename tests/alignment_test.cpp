#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <coal/alignment.hpp>

namespace coal {

TEST_CASE("align_up", "[alignment]")
{
    using tuple = std::tuple<std::size_t, std::size_t, std::size_t>;

    auto [value, align, expected] = GENERATE(
        tuple{0, 4, 0},
        tuple{3, 4, 4},
        tuple{4, 4, 4},
        tuple{5, 4, 8},

        tuple{0, 8, 0},
        tuple{7, 8, 8},
        tuple{8, 8, 8},
        tuple{9, 8, 16},

        tuple{0, 16, 0},
        tuple{15, 16, 16},
        tuple{16, 16, 16},
        tuple{17, 16, 32});

    CAPTURE(value, align, expected);

    CHECK(align_up(value, align) == expected);
}

TEST_CASE("align_down", "[alignment]")
{
    using tuple = std::tuple<std::size_t, std::size_t, std::size_t>;

    auto [value, align, expected] = GENERATE(
        tuple{0, 4, 0},
        tuple{3, 4, 0},
        tuple{4, 4, 4},
        tuple{5, 4, 4},

        tuple{0, 8, 0},
        tuple{7, 8, 0},
        tuple{8, 8, 8},
        tuple{9, 8, 8},

        tuple{0, 16, 0},
        tuple{15, 16, 0},
        tuple{16, 16, 16},
        tuple{17, 16, 16});

    CAPTURE(value, align, expected);

    CHECK(align_down(value, align) == expected);
}

} // namespace coal
