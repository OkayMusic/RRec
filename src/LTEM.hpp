#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "screencap.hpp"
#include "processing.hpp"
#include "clustering.hpp"

void handleSamLTEMAnalysis();

namespace rrec
{
void picToMat(std::string path, cv::Mat &outImg);

void showSamTheDeets(std::vector<Cluster> clusters);
} // namespace rrec
