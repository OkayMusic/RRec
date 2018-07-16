#pragma once

namespace rrec
{
cv::Mat contrast(cv::Mat image);
cv::Mat fourierTransform(cv::Mat image);
void cluster(cv::Mat image, cv::Mat brightnessScale);
}