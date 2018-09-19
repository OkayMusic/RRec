#include "cluster.hpp"

namespace rrec
{
int Cluster::getClusterNum() { return clusterNum; }

Cluster::Cluster(int N) { clusterNum = N; }

int Cluster::size() { return corePoints.size() + outerPoints.size(); }
} // namespace rrec