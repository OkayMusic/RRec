#include <opencv2/opencv.hpp>
namespace rrec
{
void basicThreshold(cv::Mat image, int cutoff)
{
    // all pixels below cutoff are set to zero, all pixels >= are set to 255
}

cv::Mat contrast(cv::Mat image)
{
    cv::Mat dest;
    // uses histogram equalization to increase contrast
    cv::cvtColor(image, dest, CV_RGB2GRAY);

    // cv::equalizeHist(image, image);
    return dest;
}
} // namespace rrec