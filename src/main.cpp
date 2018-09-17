#include <iostream>
#include <fstream>
#include <string>

#include "screencap.hpp"
#include "processing.hpp"
#include "clustering.hpp"

namespace rrec
{
void detectImage(std::string path)
{
    cv::Mat frame{cv::imread(path, CV_LOAD_IMAGE_GRAYSCALE)};

    cv::namedWindow("Original image", CV_WINDOW_AUTOSIZE);
    cv::imshow("Original image", frame);

    // equalize the colour histogram (this should make movies from different
    // sources look reasonably similar, making the code more portable)
    // additionally contrast is always increased wich is useful for processing
    cv::subtract(cv::Scalar::all(255), frame, frame);
    cv::equalizeHist(frame, frame);

    int xScale{71};
    int yScale{71};

    cv::Mat brightnessScale;
    cv::Mat localBrightness;
    cv::GaussianBlur(frame, brightnessScale, cv::Size(xScale, yScale), 0);
    cv::GaussianBlur(frame, localBrightness, cv::Size(7, 7), 0);

    cv::Mat thresh = rrec::detectSignal(localBrightness, brightnessScale);
    rrec::showDBSCAN(thresh, frame);
    cv::namedWindow("Detector", CV_WINDOW_AUTOSIZE);
    cv::imshow("Detector", frame);

    while (true)
    {
        if (cv::waitKey(30) != -1)
        {
            break;
        }
    }
}

void detectVideo(std::string path)
{
    cv::VideoCapture cap(path);

    if (!cap.isOpened())
    {
        std::cout << "Cannot open the video file. \n";
        exit(-1);
    }

    cv::namedWindow("Detector", CV_WINDOW_AUTOSIZE);
    cv::namedWindow("Original video", CV_WINDOW_AUTOSIZE);

    int secondsSkipped = 355;
    // skip the first secondsSkipped seconds of the movie
    // multiplication by 1000 to deal with seconds->milliseconds
    cap.set(CV_CAP_PROP_POS_MSEC, secondsSkipped * 1000);

    while (1)
    {
        cv::Mat frame;

        // read() decodes and captures the next frame.
        if (!cap.read(frame)) // if not success, break loop
        {
            std::cout << "\n Cannot read the video file. \n";
            exit(-1);
        }

        cv::imshow("Original video", frame);
        // now it is time to process the frame:
        // make sure the video is grayscale
        cv::cvtColor(frame, frame, CV_BGR2GRAY);

        // equalize the colour histogram (this should make movies from different
        // sources look reasonably similar, making the code more portable)
        // additionally contrast is always increased wich is useful for processing
        cv::equalizeHist(frame, frame);

        // we want the gaussian blur value at each pixel to give us a gauge of
        // relative brightness in that region of the image
        int xScale = frame.cols / 5;
        int yScale = frame.rows / 5;

        // xScale and yScale need to be odd numbers so that GaussianBlur works
        if ((xScale % 2) == 0)
            xScale += 1;
        if ((yScale % 2) == 0)
            yScale += 1;

        cv::Mat brightnessScale;
        cv::GaussianBlur(frame, brightnessScale, cv::Size(xScale, yScale), 0);

        cv::Mat localBrightness;
        cv::GaussianBlur(frame, localBrightness, cv::Size(7, 7), 0);

        cv::Mat thresh = rrec::detectSignal(localBrightness, brightnessScale);

        if (cv::waitKey(30) != -1) // kill program on any key press
        {
            break;
        }

        rrec::showDBSCAN(thresh, frame);
        cv::imshow("Detector", frame);
        std::cout << "finished frame" << std::endl;
    }
}

void removeBackground(std::string path, std::string fileName)
{
    const int rows{1296};
    const int cols{1728};

    std::ifstream inf(path, std::ios::binary);

    std::vector<float> F(rows * cols);
    float f;
    char headerByte;

    double counter;
    float maxVal{0};

    // skip the first num_header_bytes bytes
    int num_header_bytes{624};
    for (int i = 0; i < num_header_bytes; ++i)
        ;
    {
        inf.read(reinterpret_cast<char *>(&headerByte), sizeof(float));
    }

    for (int i = 0; i < rows * cols; ++i)
    {
        inf.read(reinterpret_cast<char *>(&f), sizeof(float));
        counter += static_cast<double>(f);
        F[i] = f;
        if (f > maxVal)
            maxVal = f;
    }
    double mean{counter / static_cast<double>(rows * cols)};
    std::cout << "mean: " << mean << ", maxVal: " << maxVal << std::endl;

    int cutoff = 900;
    // kill super bright background pixels
    for (int i = 0; i < rows * cols; ++i)
    {
        if (F[i] > cutoff)
            F[i] = cutoff;
    }

    // find the largest element of the array
    maxVal = 0;
    for (int i = 0; i < rows * cols; ++i)
    {
        if (F[i] > maxVal)
        {
            maxVal = F[i];
        }
    }

    // put in correct range for a cv::mat floaty image
    for (int i = 0; i < rows * cols; ++i)
    {
        F[i] = F[i] * 255.0 / maxVal;
    }

    cv::Mat img(rows, cols, CV_32FC1);

    memcpy(img.data, F.data(), F.size() * sizeof(float));
    img.convertTo(img, CV_8UC1);

    imwrite(fileName, img);
}

void picToMat(std::string path, cv::Mat outImg)
{
    const int rows{1296};
    const int cols{1728};

    std::ifstream inf(path, std::ios::binary);

    std::vector<float> F(rows * cols);
    float f;
    char headerByte;

    double counter;
    float maxVal{0};

    // skip the first num_header_bytes bytes
    int num_header_bytes{624};
    for (int i = 0; i < num_header_bytes; ++i)
        ;
    {
        inf.read(reinterpret_cast<char *>(&headerByte), sizeof(float));
    }

    for (int i = 0; i < rows * cols; ++i)
    {
        inf.read(reinterpret_cast<char *>(&f), sizeof(float));
        counter += static_cast<double>(f);
        F[i] = f;
        if (f > maxVal)
            maxVal = f;
    }
    double mean{counter / static_cast<double>(rows * cols)};
    std::cout << "mean: " << mean << ", maxVal: " << maxVal << std::endl;

    int cutoff = 900;
    // kill super bright background pixels
    for (int i = 0; i < rows * cols; ++i)
    {
        if (F[i] > cutoff)
            F[i] = cutoff;
    }

    // find the largest element of the array
    maxVal = 0;
    for (int i = 0; i < rows * cols; ++i)
    {
        if (F[i] > maxVal)
        {
            maxVal = F[i];
        }
    }

    // put in correct range for a cv::mat floaty image
    for (int i = 0; i < rows * cols; ++i)
    {
        F[i] = F[i] * 255.0 / maxVal;
    }

    memcpy(outImg.data, F.data(), F.size() * sizeof(float));
    outImg.convertTo(outImg, CV_8UC1);
}

}; // namespace rrec

void handleSamLTEMAnalysis()
{
    /*
    While(true), waits for a series of parameters for clustering and a path to
    the image which will be processed.
    */
    // make a cv::Mat which holds an image of 32bit floating point numbers
    const int rows{1296};
    const int cols{1728};

    while (1)
    {
        // get the path to the image to be analyzed
        std::string path;
        std::cin >> path;

        if (path.compare("quit"))
        {
            exit(0);
        }

        // // get the clustering parameters
        // int L; // lengthscale over which brightness varies
        // int d; // lengthscale over which signal is detected
        // double sigma;
        //
        // std::cin >> L;

        cv::Mat currentImage(rows, cols, CV_32FC1);

        // pass the matrix to picToMat, which will load and convert currentImage
        // std::string path = "../binary_data/fr_50001.pic";
        rrec::picToMat(path, currentImage);

        cv::imwrite("testy2.jpg", currentImage);
    }
}

int main()
{
    // std::string path = "../video/LTEM_cam.mpg";
    // std::cout << path << std::endl;
    // rrec::detectVideo(path);

    handleSamLTEMAnalysis();

    return 0;
}