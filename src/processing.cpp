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
    cv::Mat padded; //expand input image to optimal size
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

#define MAXCLUSTERSIZE 10000

std::vector<std::vector<int>> clusterPoints(MAXCLUSTERSIZE,
                                            std::vector<int>(2));

// we use this enum to label points
enum
{
    unlabelled,
    noise,
    TBD, // TBD label implies 'it is perimeter or core, soon to be determined'
    perimeter,
    core
};

void getNeighbours(std::vector<std::vector<bool>> &thresh,
                   std::vector<std::vector<int>> &neighbours,
                   int i, int j, int eps)
{
    // there can be no core nodes on the edge of an image
    if (i == 0 ||
        j == 0 ||
        i == (thresh.size() - 1) ||
        j == (thresh[i].size() - 1))
    {
        neighbours[0][0] = -1;
        return;
    }

    if (thresh[i - 1][j])
    {

        neighbours[0][0] = i - 1;
        neighbours[0][1] = j;
    }
    else
    {
        neighbours[0][0] = -1;
        return;
    }
    if (thresh[i][j - 1])
    {

        neighbours[1][0] = i;
        neighbours[1][1] = j - 1;
    }
    else
    {
        neighbours[0][0] = -1;
        return;
    }
    if (thresh[i + 1][j])
    {

        neighbours[2][0] = i + 1;
        neighbours[2][1] = j;
    }
    else
    {
        neighbours[0][0] = -1;
        return;
    }
    if (thresh[i][j + 1])
    {

        neighbours[3][0] = i;
        neighbours[3][1] = j + 1;
    }
    else
    {
        neighbours[0][0] = -1;
        return;
    }
}

void getCluster(std::vector<std::vector<bool>> &thresh,
                std::vector<std::vector<int>> &pointFlags,
                int i, int j, int eps, int minPts)
{
    static long long int clusterNum; // keeps track of which cluster we're up to
    int N = 0;                       // number of points currently in cluster
    int clusterStride = 2;

    std::vector<std::vector<int>> neighbours(4, std::vector<int>(2));
    getNeighbours(thresh, neighbours, i, j, eps);

    if (neighbours[0][0] == -1)
    {
        pointFlags[i][j] = noise;
        return;
    }

    pointFlags[i][j] = core;
    // first label all points in neighbours as TBD if they were previously
    // unlabelled
    for (auto points : neighbours)
    {
        N += 1;
        if (pointFlags[points[0]][points[1]] == unlabelled)
            pointFlags[points[0]][points[1]] = TBD;
    }

    // add the neighbours to clusterPoints
    for (int i = 0; i < 4; ++i)
    {
        clusterPoints[i] = neighbours[i];
    }

    // now loop over all neighbours that could be in the cluster:
    // if a neighbour already has a noise label, relabel as perimeter
    // if a neighbour is unlabelled, check to see if it is a core node
    // if the neighbour is a core node, add its neighbours to neighbours
    // if the core node's neighbours are unlabelled, label them as TBD
    // if the neighbour is not core, label it as perimeter
    for (int a = 0; a < N; ++a)
    {
        int x = clusterPoints[a][0];
        int y = clusterPoints[a][1];
        if (pointFlags[x][y] == noise)
        {
            pointFlags[x][y] = perimeter + clusterNum;
        }

        if (pointFlags[x][y] == TBD)
        {
            getNeighbours(thresh, neighbours, x, y, eps);

            // the next line can be read as 'if [x, y] is a core node'
            if (neighbours[0][0] != -1)
            {
                // if execution reached here, [x, y] is a core node point
                pointFlags[x][y] = core + clusterNum;
                for (auto coords : neighbours)
                {
                    if (pointFlags[coords[0]][coords[1]] == unlabelled)
                    {
                        pointFlags[coords[0]][coords[1]] = TBD;
                        clusterPoints[N] = coords;
                        N += 1;
                    }
                }
            }
            else
            {
                pointFlags[x][y] = perimeter + clusterNum;
            }
        }
    }

    // incrament the cluster number!
    clusterNum += clusterStride;
}

void dbscan(std::vector<std::vector<bool>> &thresh,
            std::vector<std::vector<int>> &pointFlags)
{
    // eps must be an odd integer
    int eps = 3;
    // set minPts = every neighbour - this allows for further optimizations!
    int minPts = 4;

    int rows = thresh.size();
    int cols = thresh[0].size();

    // do the clustering
    for (int i = 0; i < rows; ++i)
    {
        // std::cout << "Clustering about row: " << i << std::endl;
        for (int j = 0; j < cols; ++j)
        {
            if (thresh[i][j] && (pointFlags[i][j] == unlabelled))
            {
                getCluster(thresh, pointFlags, i, j, eps, minPts);
            }
            if (!thresh[i][j] && pointFlags[i][j] == unlabelled)
            {
                pointFlags[i][j] = noise;
            }
        }
    }
}

void showDBSCAN(cv::Mat threshold, cv::Mat origImg)
{
    /*
        Takes a threshold generated by detectSignal and returns a cv::Mat of the
        original image with rectangles overlayed on top of detected skyrmions
    */
    int rows = threshold.rows;
    int cols = threshold.cols;

    std::vector<std::vector<bool>> thresh(rows, std::vector<bool>(cols));
    std::vector<std::vector<int>>
    pointFlags(rows, std::vector<int>(cols, unlabelled));

    // now we want to populate the bool vector
    unsigned char *thresholdPointer;
    for (int i = 0; i < rows; ++i)
    {
        thresholdPointer = threshold.ptr<unsigned char>(i);
        for (int j = 0; j < cols; ++j)
        {
            if (thresholdPointer[j] == 0)
                thresh[i][j] = false;
            else
                thresh[i][j] = true;
        }
    }

    dbscan(thresh, pointFlags);

    // now we have the thresholded image we can redraw image to show clustering
    unsigned char *imgPointer;

    for (int i = 0; i < origImg.rows; ++i)
    {
        imgPointer = origImg.ptr<unsigned char>(i);
        for (int j = 0; j < origImg.cols; ++j)
        {
            if (pointFlags[i][j] == noise)
                imgPointer[j] = 0;
            else if (pointFlags[i][j] % 2 == perimeter % 2)
                imgPointer[j] = 255;
            else
                imgPointer[j] = 128;
        }
    }
}

} // namespace rrec