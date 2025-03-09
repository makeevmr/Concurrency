#include "../source/table.hpp"
#include "../source/philosopher.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/body/inject_fault.hpp>
#include <twist/test/body/wg.hpp>

#include <vector>

using namespace dining;  // NOLINT

TEST_SUITE(Dining) {
  TWIST_STRESS_TEST(StressDining5, 10s) {
    static const size_t kSeats = 5;

    Table table{kSeats};

    std::vector<Philosopher> philosophers;
    for (size_t seat = 0; seat < kSeats; ++seat) {
      philosophers.emplace_back(table, seat);
    }

    twist::test::body::WaitGroup wg;

    wg.Add(kSeats, [&](size_t seat) {
      auto& plato = philosophers.at(seat);

      course::test::TimeBudget budget;
      while (budget) {
        plato.Eat();
        plato.Think();
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
