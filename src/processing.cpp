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

// if MAXMINPTS is defined, optimizations are made which are only available if
// minpts = clusterSize^2
#define MAXMINPTS

// we use this enum to label points
enum
{
    unlabelled,
    noise,
    perimeter,
    core
};

std::vector<std::vector<int>>
getNeighbours(std::vector<std::vector<bool>> *thresh, int i, int j,
              int clusterSize)
{
    // tracks the number of truthy neighbours found thus far
    int neighbourNum = 0;
    std::vector<std::vector<int>> neighbours(25, std::vector<int>(2));

    // where we start/stop the loop
    int a_start = i - clusterSize / 2;
    int a_stop = i + clusterSize / 2;
    int b_start = j - clusterSize / 2;
    int b_stop = j + clusterSize / 2;

#ifndef MAXMINPTS
    // check to see if we're on the edge - if we are, prevent segfault
    if (i < clusterSize / 2)
        a_start = 0;
    if (j < clusterSize / 2)
        b_start = 0;
    if (i > ((*thresh).size() - clusterSize / 2 - 1))
        a_stop = (*thresh).size() - 1;
    if (j > ((*thresh)[i].size() - clusterSize / 2 - 1))
        b_stop = (*thresh)[i].size() - 1;

    // get the truthy units in cluster
    for (int a = a_start; a <= a_stop; ++a)
    {
        for (int b = b_start; b <= b_stop; ++b)
        {
            if ((*thresh)[a][b])
            {
                neighbours[neighbourNum++] = std::vector<int>{a, b};
            }
        }
    }

    // finally resize neighbours to be the correct size
    neighbours.resize(neighbourNum);

#else

    std::vector<std::vector<int>> emptyVector;

    int halfCSize = clusterSize / 2;

    // there can be no core nodes on the edge of an image
    if (i < halfCSize ||
        j < halfCSize ||
        i > ((*thresh).size() - halfCSize - 1) ||
        j > ((*thresh)[i].size() - halfCSize - 1))
        return emptyVector;

    // check to see if we're in a cluster
    for (int a = a_start; a <= a_stop; ++a)
    {
        for (int b = b_start; b <= b_stop; ++b)
        {
            if ((*thresh)[a][b])
            {
                neighbours[neighbourNum][0] = a;
                neighbours[neighbourNum++][1] = b;
            }
            else
                return emptyVector;
        }
    }

#endif
    return neighbours;
}

void getCluster(std::vector<std::vector<bool>> *thresh,
                std::vector<std::vector<int>> *pointFlags,
                int i, int j, int clusterSize, int minPts)
{
    static long long int clusterNum; // keeps track of which cluster we're up to
    int clusterStride = 2;

    std::vector<std::vector<int>> neighbours; // this will be used later
    std::vector<std::vector<int>> clusterPoints = getNeighbours(thresh, i, j,
                                                                clusterSize);

    if (clusterPoints.size() < minPts)
    {
        (*pointFlags)[i][j] = noise;
        return;
    }

    // now loop over all neighbours that could be in the cluster:
    // if a neighbour already has a noise label, relabel as perimeter
    // if a neighbour is unlabelled, check to see if it is a core node
    // if the neighbour is a core node, add its neighbours to neighbours
    // if the neighbour is not core, label it as perimeter

    for (int a = 0; a < clusterPoints.size(); ++a)
    {
        int x = clusterPoints[a][0];
        int y = clusterPoints[a][1];
        if ((*pointFlags)[x][y] == noise)
        {
            (*pointFlags)[x][y] = perimeter + clusterNum;
        }

        if ((*pointFlags)[x][y] == unlabelled)
        {
            neighbours = getNeighbours(thresh, x, y, clusterSize);
            if (neighbours.size() >= minPts)
            {
                // if execution reached here, [x, y] is a core node point
                (*pointFlags)[x][y] = core + clusterNum;
                for (auto coords : neighbours)
                {
                    if ((*pointFlags)[coords[0]][coords[1]] != unlabelled)
                        continue;
                    if (std::find(clusterPoints.begin() + a, clusterPoints.end(),
                                  coords) == clusterPoints.end())
                    {
                        clusterPoints.push_back(coords);
                    }
                }
            }
            else
            {
                (*pointFlags)[x][y] = perimeter + clusterNum;
            }
        }
    }

    // incrament the cluster number!
    clusterNum += clusterStride;
}

std::vector<std::vector<int>> dbscan(std::vector<std::vector<bool>> thresh)
{
    /*
        Takes a 2D boolean vector, returns 2D vector of cluster locations.
    */
    // clusterSize must be an odd integer
    int clusterSize = 3;
    // set minPts = every neighbour - this allows for further optimizations!
    int minPts = clusterSize * clusterSize;

    // the return vector
    std::vector<std::vector<int>>
    pointFlags(thresh.size(), std::vector<int>(thresh[0].size()));

    // initialize the return vector to be made up entirely of unlabelled points
    for (int i = 0; i < thresh.size(); ++i)
    {
        for (int j = 0; j < thresh[i].size(); ++j)
        {
            pointFlags[i][j] = unlabelled;
        }
    }

    // do the clustering
    for (int i = 0; i < thresh.size(); ++i)
    {
        std::cout << "Clustering about row: " << i << std::endl;
        for (int j = 0; j < thresh[i].size(); ++j)
        {
            if (thresh[i][j] && (pointFlags[i][j] == unlabelled))
            {
                getCluster(&thresh, &pointFlags, i, j, clusterSize, minPts);
            }
            if (!thresh[i][j])
            {
                pointFlags[i][j] = noise;
            }
        }
    }

    return pointFlags;
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

    std::vector<std::vector<int>> cluster = dbscan(thresh);

    // for (auto i : cluster)
    // {
    //     for (auto j : i)
    //         std::cout << j << " ";
    //     std::cout << std::endl;
    // }

    // now we have the thresholded image we can redraw image to show clustering
    unsigned char *imgPointer;

    for (int i = 0; i < origImg.rows; ++i)
    {
        imgPointer = origImg.ptr<unsigned char>(i);
        for (int j = 0; j < origImg.cols; ++j)
        {
            if (cluster[i][j] == 1)
                imgPointer[j] = 0;
            else if (cluster[i][j] % 2 == 0)
                imgPointer[j] = 255;
            else
                imgPointer[j] = 128;
        }
    }
}

} // namespace rrec