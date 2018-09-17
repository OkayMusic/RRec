#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <algorithm>

namespace rrec
{

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
    cv::Mat padded; // expand input image to optimal size
    int m = cv::getOptimalDFTSize(image.rows);
    int n = cv::getOptimalDFTSize(image.cols);
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

cv::Mat detectSignal(cv::Mat image, cv::Mat brightnessScale)
{
    /*
        Takes an image and a brightness scale as inputs. The brightness scale
        is used to threshold the image, and the thresholded image is returned.
    */
    int rows = image.rows;
    int cols = image.cols;

    if (rows != brightnessScale.rows || cols != brightnessScale.cols)
    {
        std::cout << "Invalid image or brightness scale" << std::endl;
        exit(-1);
    }

    // create the cv::Mat that we will return
    cv::Mat threshold;
    image.copyTo(threshold); // this step just ensures that we have allocated
                             // the correct amount of memory

    // if all images are stored continuously in memory then we need only grab a
    // uchar* pointer once per image
    if (image.isContinuous() && brightnessScale.isContinuous() &&
        threshold.isContinuous())
    {
        cols *= rows;
        rows = 1;
    }

    // 256 channel colour <=> unsigned char
    unsigned char *imgPointer;
    unsigned char *brightnessPointer;
    unsigned char *thresholdPointer;

    // the mean of the uniform distribution the original picture was eq'd to
    double mu = 127.5;

    for (int i = 0; i < rows; ++i)
    {
        imgPointer = image.ptr<unsigned char>(i);
        brightnessPointer = brightnessScale.ptr<unsigned char>(i);
        thresholdPointer = threshold.ptr<unsigned char>(i);

        for (int j = 0; j < cols; ++j)
        {
            double difference;
            double stdDev;
            if (brightnessPointer[j] > mu)
            {
                difference = brightnessPointer[j] - mu;
            }
            else
            {
                difference = -brightnessPointer[j] + mu;
            }

            // the standard deviation of the colours in the entire image is
            // 73.9. If brightnessPointer[j] != 127.5, then we are sampling
            // from some distribution which has been locally shifted. Whatever
            // distribution it is, we know that if brightnessPointer[j] = 255||0
            // then D = 127.5 and the standard deviation of the local
            // distribution is 0, as all pixels are the same and the
            // distribution is a delta function. For simplicity I linearly
            // interpolate the standard deviation for all other values, and set
            // the threshold value accordingly (although the exact stddev could
            // be calculated exactly for each pixel that would be far too
            // costly)
            stdDev = 73.9 * (1 - difference / 127.5);

            if (imgPointer[j] > brightnessPointer[j] + stdDev / 2)
                thresholdPointer[j] = 255;
            else
                thresholdPointer[j] = 0;
        }
    }
    return threshold;
}

} // namespace rrec