#pragma once

#include <vector>
struct Ratio {
  int getItemRatio;
  int addRatio;
  int containsRatio;
  int removeRatio;
  int clearRatio;
};

enum class METHOD {
  GET_ITEM,
  ADD_ITEM,
  CONTAINS_ITEM,
  REMOVE_ITEM,
  NUM_METHODS, // 4 methods
};

constexpr int RATIO_MAX = 100;

namespace Schedule {
[[nodiscard]] const std::vector<METHOD> buildSchedule(const Ratio &ratio, const int testIterations);
[[nodiscard]] const std::vector<METHOD> buildSchedule(const METHOD method, const int testIterations);
[[nodiscard]] const Ratio transformRatios(const Ratio &ratio);
} // namespace Schedule
