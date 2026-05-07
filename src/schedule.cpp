#include "schedule.h"
#include <cassert>
#include <random>
#include <vector>

[[nodiscard]] const std::vector<METHOD>
Schedule::buildSchedule(const Ratio &ratio, const int testIterations) {
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> distr{0, RATIO_MAX - 1};

  const Ratio newRatio = transformRatios(ratio);
  std::vector<METHOD> schedule;
  schedule.reserve(testIterations);
  assert((int)schedule.capacity() == testIterations);
  int rand{};

  for (auto i{0}; i < testIterations; i++) {
    rand = distr(gen);

    if (rand < newRatio.getItemRatio)
      schedule.emplace_back(METHOD::GET_ITEM);

    else if (rand < newRatio.addRatio)
      schedule.emplace_back(METHOD::ADD_ITEM);

    else if (rand < newRatio.containsRatio)
      schedule.emplace_back(METHOD::CONTAINS_ITEM);

    else
      schedule.emplace_back(METHOD::REMOVE_ITEM);
  }

  return schedule;
}

[[nodiscard]] const std::vector<METHOD>
Schedule::buildSchedule(const METHOD method, const int testIterations) {
  std::vector<METHOD> schedule;
  schedule.reserve(testIterations);
  assert((int)schedule.capacity() == testIterations);

  for (auto i{0}; i < testIterations; i++)
    schedule.emplace_back(method);

  return schedule;
}

// Map ratios to a set of values 0 - 100.
[[nodiscard]] const Ratio Schedule::transformRatios(const Ratio &ratio) {
  int getItemRatio{ratio.getItemRatio};
  int addRatio{getItemRatio + ratio.addRatio};
  int containsRatio{addRatio + ratio.containsRatio};
  int removeRatio{containsRatio + ratio.removeRatio};

  assert(removeRatio == RATIO_MAX);

  return Ratio{getItemRatio, addRatio, containsRatio, removeRatio};
}
