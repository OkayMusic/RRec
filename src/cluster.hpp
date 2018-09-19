#pragma once

#include <vector>
#include <array>

namespace rrec
{
class Cluster
{
  private:
    int clusterNum; // unique cluster ID

  public:
    int getClusterNum();

    std::vector<std::array<int, 2>> corePoints;  // cluster's core points
    std::vector<std::array<int, 2>> outerPoints; // points on perimeter of cluster

    Cluster(int N);

    int size();
};
} // namespace rrec