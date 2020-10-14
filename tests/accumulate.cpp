#include "catch.hpp"

#include "pipes/pipes.hpp"

#include <vector>

namespace {
    struct Sentinal {
        int value;
        int copies = 0;
        int moves = 0;

        explicit Sentinal(int value, int copies = 0, int moves = 0)
            : value(value), copies(copies), moves(moves) {}

        Sentinal(const Sentinal& rhs)
            : value(rhs.value)
            , copies(rhs.copies + 1)
            , moves(rhs.moves) {}

        Sentinal(Sentinal&& rhs)
            : value(rhs.value)
            , copies(rhs.copies)
            , moves(rhs.moves + 1) {}

        Sentinal& operator=(const Sentinal& rhs) {
            value = rhs.value;
            copies = rhs.copies + 1;
            moves = rhs.moves;
            return *this;
        } 

        Sentinal& operator=(Sentinal&& rhs) {
            value = rhs.value;
            copies = rhs.copies;
            moves = rhs.moves + 1;
            return *this;
        } 

        friend bool operator==(const Sentinal& lhs, const Sentinal& rhs) {
            return std::tie(lhs.value, lhs.copies, lhs.moves)
                == std::tie(rhs.value, rhs.copies, rhs.moves);
        }

        friend std::ostream& operator<<(std::ostream& os, const Sentinal& sent) {
            return os << "(" << sent.value << ", " << sent.copies <<" copies, " << sent.moves << " moves)";
        }
    };
}

TEST_CASE("accumulate")
{
    const std::vector<int> v = {1, 2, 3};

    REQUIRE((v >>= pipes::accumulate(0, [](int acc, int ele) { return acc + ele; })) == 6);
}

TEST_CASE("accumulate re-use")
{
    const std::vector<int> v = {1, 2, 3};
    const std::vector<int> v2 = {4, 5, 6};

    auto accumulator = pipes::accumulate(0, [](int acc, int ele) { return acc + ele; });

    REQUIRE((v >>= accumulator) == 6);
    REQUIRE((v >>= accumulator) == 12);
    REQUIRE((v2 >>= accumulator) == 27);
    REQUIRE(accumulator.sink() == 27);
}

TEST_CASE("accumulate move only")
{
    const std::vector<int> v = {1, 2, 3};

    SECTION("direct")
    {
        const auto result = v 
            >>= pipes::accumulate(std::make_unique<int>(0), 
                    [](auto acc, int ele) { *acc += ele; return acc; });

        REQUIRE(*result == 6);
    }

    SECTION("generic pipe")
    {
        const auto result = v 
            >>= pipes::filter([](const auto&) { return true; })
            >>= pipes::accumulate(std::make_unique<int>(0), 
                    [](auto acc, int ele) { *acc += ele; return acc; });

        REQUIRE(*result == 6);
    }

    SECTION("composite pipe")
    {
        const auto result = v 
            >>= (pipes::filter([](const auto&) { return true; })
                    >>= pipes::filter([](const auto&) { return true; }))
            >>= pipes::accumulate(std::make_unique<int>(0), 
                    [](auto acc, int ele) { *acc += ele; return acc; });

        REQUIRE(*result == 6);
    }
}

TEST_CASE("accumulate minimal moves")
{
    const std::vector<int> v = {1, 2, 3};

    SECTION("direct")
    {
        const auto result = v
            >>= pipes::accumulate(Sentinal(0), 
                [](Sentinal&& acc, int ele) -> Sentinal&& { acc.value += ele; return std::move(acc); });
        // 0 copies
        // 5 moves: into accumulate_pipe, assign 3 times in loop, into result
        REQUIRE(result == Sentinal(6, 0, 5));
    }

    SECTION("generic pipe")
    {
        const auto result = v
            >>= pipes::filter([](const auto&) { return true; })
            >>= pipes::accumulate(Sentinal(0), 
                [](Sentinal&& acc, int ele) -> Sentinal&& { acc.value += ele; return std::move(acc); });
        // 0 copies
        // 6 moves: same as above, into filter_pipe
        REQUIRE(result == Sentinal(6, 0, 6));
    }

    SECTION("composite pipe")
    {
        const auto result = v
            >>= (pipes::filter([](const auto&) { return true; }) 
                >>= pipes::filter([](const auto&) { return true; }))
            >>= pipes::accumulate(Sentinal(0), 
                [](Sentinal&& acc, int ele) -> Sentinal&& { acc.value += ele; return std::move(acc); });
        // 0 copies
        // 7 moves: same as above, into 2nd filter pipe
        REQUIRE(result == Sentinal(6, 0, 7));
    }
}

TEST_CASE("to_vector")
{
    const std::vector<int> v = {1, 2, 3};

    REQUIRE((v >>= pipes::to_vector<int>()) == v);

    SECTION("implicit conversion")
    {
        REQUIRE((v >>= pipes::to_vector<float>()) == std::vector<float>{1.0f, 2.0f, 3.0f});
    }
}

TEST_CASE("to_vector re_use")
{
    const std::vector<int> v = {1, 2, 3};

    auto accumulator = pipes::to_vector<int>();

    REQUIRE((v >>= accumulator) == v);
    REQUIRE((v >>= accumulator) == std::vector<int>{1, 2, 3, 1, 2, 3});

    REQUIRE(accumulator.sink() == std::vector<int>{1, 2, 3, 1, 2, 3});
    REQUIRE(std::move(accumulator).sink() == std::vector<int>{1, 2, 3, 1, 2, 3});

    // Moved from empties accumulator
    REQUIRE(accumulator.sink() == std::vector<int>{});

    // Refill and empty again
    REQUIRE((v >>= std::move(accumulator)) == v);
    REQUIRE(accumulator.sink() == std::vector<int>{});
}

TEST_CASE("count")
{
    const std::vector<int> v = {1, 2, 3};

    REQUIRE((v >>= pipes::count()) == v.size());

    SECTION("filter")
    {
        const std::size_t result = v >>= pipes::filter([](int x) { return x % 2 == 1; }) >>= pipes::count();
        REQUIRE(result == 2);
    }

    SECTION("count int")
    {
        const int result = v >>= pipes::count<int>();
        REQUIRE(result == 3);
    }
}
