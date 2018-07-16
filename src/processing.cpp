#include <opencv2/opencv.hpp>
namespace rrec
{
void basicThreshold(cv::Mat image)
{
    // all imgPointers below cutoff are set to zero, all imgPointers >= are set to 255
}

cv::Mat contrast(cv::Mat image)
{
    cv::Mat dest;
    // uses histogram equalization to increase contrast
    cv::cvtColor(image, dest, CV_RGB2GRAY);

    // cv::equalizeHist(image, image);
    return dest;
}

cv::Mat fourierTransform(cv::Mat image)
{
    cv::Mat padded; //expand input image to optimal size
    int m = cv::getOptimalDFTSize(image.rows);
    int n = cv::getOptimalDFTSize(image.cols); // on the border add zero imgPointers
    copyMakeBorder(image, padded, 0, m - image.rows, 0, n - image.cols,
                   cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(
                                                     padded.size(), CV_32F)};
    cv::Mat complexI;
    merge(planes, 2, complexI); // Add to the expanded another plane with zeros

    dft(complexI, complexI); // this way the result may fit in the source matrix
    split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))

    magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
    cv::Mat magI;
    planes[0].copyTo(magI);

    magI += cv::Scalar::all(1); // switch to logarithmic scale
    log(magI, magI);

    magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));
    int cx = magI.cols / 2;
    int cy = magI.rows / 2;

    cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left
    cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

    cv::Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);

    normalize(magI, magI, 0, 1, CV_MINMAX);
    return magI;
}

void cluster(cv::Mat image, cv::Mat brightnessScale)
{
    int rows = image.rows;
    int cols = image.cols;

    if (rows != brightnessScale.rows || cols != brightnessScale.cols)
    {
        std::cout << "Invalid brightness scale" << std::endl;
        exit(-1);
    }

    // if both images are stored continuously in memory then we need only grab a
    // uchar* pointer once per image
    if (image.isContinuous() && brightnessScale.isContinuous())
    {
        cols *= rows;
        rows = 1;
    }

    unsigned char *imgPointer;
    unsigned char *brightnessPointer;

    for (int i = 0; i < rows; ++i)
    {
        imgPointer = image.ptr<unsigned char>(i);
        brightnessPointer = brightnessScale.ptr<unsigned char>(i);

        for (int j = 0; j < cols; ++j)
        {
            // do something
        }
    }
}

} // namespace rrec