#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

#include "cluster.hpp"

namespace rrec
{
std::vector<Cluster> showDBSCAN(cv::Mat threshold, cv::Mat origImg);
} // namespace rrec