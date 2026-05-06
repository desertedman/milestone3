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
};

constexpr int RATIO_MAX = 100;

namespace Schedule {
const std::vector<METHOD> buildSchedule(const Ratio &ratio);
const Ratio transformRatios(const Ratio &ratio);
} // namespace Schedule
