#include <iostream>
#include <fstream>
#include <string>

#include "screencap.hpp"
#include "processing.hpp"
#include "clustering.hpp"

namespace rrec
{
void picToMat(std::string path, cv::Mat &outImg)
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
    // std::cout << "mean: " << mean << ", maxVal: " << maxVal << std::endl;

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

void showSamTheDeets(std::vector<Cluster> clusters)
{
    for (Cluster cluster : clusters)
    {
        // first give sam the core points
        std::cout << "core_points: ";
        for (auto coord : cluster.corePoints)
        {
            std::cout << coord[0] << " " << coord[1] << ",";
        }
        std::cout << ";";

        // now give sam the points on the cluster's perimeter
        std::cout << "outer_points: ";
        for (auto coord : cluster.outerPoints)
        {
            std::cout << coord[0] << " " << coord[1] << ",";
        }
        std::cout << ";";

        // also give the size of the cluster
        std::cout << "cluster_size: " << cluster.size();
    }
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

        // get the clustering parameters
        int L;        // lengthscale over which brightness varies
        int d;        // lengthscale over which signal is detected
        double sigma; // CURRENTLY UNUSED!!

        std::cin >> L;
        std::cin >> d;
        std::cin >> sigma;

        // make sure that L and d are odd numbers
        if (L % 2 == 0)
            L += 1;
        if (d % 2 == 0)
            d += 1;

        // make the matrix which will hold the image requested by the user
        cv::Mat frame(rows, cols, CV_32FC1);

        // pass the matrix to picToMat, which will load and convert currentImage
        rrec::picToMat(path, frame);

        // equalize the colour histogram (this should make movies from different
        // sources look reasonably similar, making the code more portable)
        // additionally contrast is always increased wich is useful for processing
        cv::equalizeHist(frame, frame);

        // we want the gaussian blur value at each pixel to give us a gauge of
        // relative brightness in that region of the image
        cv::Mat brightnessScale;
        cv::GaussianBlur(frame, brightnessScale, cv::Size(L, L), 0);

        cv::Mat localBrightness;
        cv::GaussianBlur(frame, localBrightness, cv::Size(d, d), 0);

        // perform the statistical analysis
        cv::Mat thresh = rrec::detectSignal(localBrightness, brightnessScale);

        std::vector<rrec::Cluster> output = rrec::showDBSCAN(thresh, frame);

        // finally, shoot the results off to sam
        rrec::showSamTheDeets(output);
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