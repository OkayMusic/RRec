#pragma once

#include <vector>
#include <array>
#include <opencv2/opencv.hpp>

#include "cluster.hpp"

namespace rrec
{
class DBSCAN
{
  private:
    std::vector<std::array<int, 2>> clusterPoints;
    std::vector<std::array<int, 2>> perimPoints;
    std::vector<std::array<int, 2>> corePoints;

    // we use this enum to label points
    enum dbscan_labels
    {
        unlabelled,
        noise,
        TBD, // TBD label => 'it is perimeter or core, soon to be determined'
        perimeter,
        core
    };

    void getNeighbours(std::vector<std::vector<bool>> &thresh,
                       std::vector<std::array<int, 2>> &neighbours,
                       int i, int j, int eps);

    Cluster getCluster(std::vector<std::vector<bool>> &thresh,
                       std::vector<std::vector<int>> &pointFlags,
                       int i, int j, int eps, int minPts);

    std::vector<Cluster> do_dbscan(std::vector<std::vector<bool>> &thresh,
                                   std::vector<std::vector<int>> &pointFlags);

  public:
    DBSCAN(int numPixels); // DBSCAN should be told the # of pixels in the img

    std::vector<Cluster> getClusters(cv::Mat threshold, cv::Mat outImage);
};
} // namespace rrec