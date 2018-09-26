#include "dbscan.hpp"

namespace rrec
{

void DBSCAN::getNeighbours(std::vector<std::vector<bool>> &thresh,
                           std::vector<std::array<int, 2>> &neighbours,
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

Cluster DBSCAN::getCluster(std::vector<std::vector<bool>> &thresh,
                           std::vector<std::vector<int>> &pointFlags,
                           int i, int j, int eps, int minPts)
{
    static long long int clusterNum; // keeps track of which cluster we're up to
    int N = 0;                       // number of points currently in cluster
    int coreCount = 0;
    int perimCount = 0;
    int clusterStride = 2;

    std::vector<std::array<int, 2>> neighbours(4);
    getNeighbours(thresh, neighbours, i, j, eps);

    if (neighbours[0][0] == -1)
    {
        pointFlags[i][j] = noise;
        return Cluster(-1);
    }

    pointFlags[i][j] = core;
    corePoints[coreCount][0] = i;
    corePoints[coreCount][1] = j;
    ++coreCount;

    Cluster cluster(clusterNum);

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
            perimPoints[perimCount][0] = x;
            perimPoints[perimCount][1] = y;
            ++perimCount;
        }

        if (pointFlags[x][y] == TBD)
        {
            getNeighbours(thresh, neighbours, x, y, eps);

            // the next line can be read as 'if [x, y] is a core node'
            if (neighbours[0][0] != -1)
            {
                // if execution reached here, [x, y] is a core node point
                pointFlags[x][y] = core + clusterNum;
                corePoints[coreCount][0] = x;
                corePoints[coreCount][1] = y;
                ++coreCount;
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
                perimPoints[perimCount][0] = x;
                perimPoints[perimCount][1] = y;
                ++perimCount;
            }
        }
    }

    // now we have all the cluster details, we should finish making the Cluster
    // object - first populating the outerPoints vector
    cluster.outerPoints.resize(perimCount);
    for (int i = 0; i < perimCount; ++i)
    {
        cluster.outerPoints[i][0] = perimPoints[i][0];
        cluster.outerPoints[i][1] = perimPoints[i][1];
    }

    // now populate corePoints
    cluster.corePoints.resize(coreCount);
    for (int i = 0; i < coreCount; ++i)
    {
        cluster.corePoints[i][0] = corePoints[i][0];
        cluster.corePoints[i][1] = corePoints[i][1];
    }

    // incrament the cluster number!
    clusterNum += clusterStride;

    return cluster;
}

std::vector<Cluster> DBSCAN::do_dbscan(std::vector<std::vector<bool>> &thresh,
                                       std::vector<std::vector<int>> &pointFlags)
{
    // eps must be an odd integer
    int eps = 3;
    // set minPts = every neighbour - this allows for further optimizations!
    int minPts = 4;

    int rows = thresh.size();
    int cols = thresh[0].size();

    std::vector<Cluster> clusters;
    // do the clustering
    for (int i = 0; i < rows; ++i)
    {
        // std::cout << "Clustering about row: " << i << std::endl;
        for (int j = 0; j < cols; ++j)
        {
            if (thresh[i][j] && (pointFlags[i][j] == unlabelled))
            {
                Cluster cluster = getCluster(thresh, pointFlags, i, j, eps,
                                             minPts);
                if (cluster.getClusterNum() != -1)
                    clusters.push_back(cluster);
            }
            if (!thresh[i][j] && pointFlags[i][j] == unlabelled)
            {
                pointFlags[i][j] = noise;
            }
        }
    }
    return clusters;
}

DBSCAN::DBSCAN(int numpixels) : clusterPoints(numpixels),
                                perimPoints(numpixels),
                                corePoints(numpixels)
{
}

std::vector<Cluster> DBSCAN::getClusters(cv::Mat threshold, cv::Mat outImage)
{

    /*
        Takes a threshold generated by detectSignal and returns a cv::Mat of the
        original image with rectangles overlayed on top of detected skyrmions
    */
    size_t rows = threshold.rows;
    size_t cols = threshold.cols;

    // make sure outImage is the correct size
    outImage.create(rows, cols, CV_8UC1);

    std::vector<std::vector<bool>> thresh{rows, std::vector<bool>(cols)};
    std::vector<std::vector<int>>
        pointFlags{rows, std::vector<int>(cols, unlabelled)};

    // now we want to populate the bool vector
    unsigned char *thresholdPointer;
    for (int i = 0; i < rows; ++i)
    {
        thresholdPointer = threshold.ptr<unsigned char>(i);
        for (int j = 0; j < cols; ++j)
        {
            if (thresholdPointer[j] < 128)
                thresh[i][j] = false;
            else
                thresh[i][j] = true;
        }
    }

    std::vector<Cluster> clusters{do_dbscan(thresh, pointFlags)};

    // do some additional pruning: kill very big/very small clusters
    // in order to do this it can be useful to calculate some statistics first
    int min{clusters[0].size()}, max{clusters[0].size()};
    double mean{0};
    for (Cluster cluster : clusters)
    {
        if (cluster.size() > max)
            max = cluster.size();
        else if (cluster.size() < min)
            min = cluster.size();

        mean += cluster.size();
    }
    mean /= clusters.size();

    double stddev{0};
    // now work out the average deviation from the mean
    for (Cluster cluster : clusters)
    {
        stddev += (cluster.size() - mean) * (cluster.size() - mean);
    }
    stddev /= clusters.size();
    stddev = std::sqrt(stddev);

    // std::cout << "stddev: " << stddev << ", mean: " << mean << std::endl;
    // std::cout << "min: " << min << ", max: " << max << std::endl;

    // now work out which clusters are anomalous, saving the good'uns in
    // finalClusters
    std::vector<Cluster> finalClusters;
    finalClusters.reserve(clusters.size());
    int upperBound = mean + stddev;
    int lowerBound = 4;
    for (int i = 0; i < clusters.size(); ++i)
    {
        if (clusters[i].size() > upperBound ||
            clusters[i].size() < lowerBound)
        {
            // if the cluster is too big/too small, remove it from image
            // the image will be generated from pointFlags, so remove from there
            for (auto coords : clusters[i].corePoints)
            {
                pointFlags[coords[0]][coords[1]] = noise;
            }
            for (auto coords : clusters[i].outerPoints)
            {
                if (clusters[i].size() > 2 * stddev + mean)
                {
                    pointFlags[coords[0]][coords[1]] = noise;
                }
                // pointFlags[coords[0]][coords[1]] = noise;
            }
        }
        else
        {
            // if the cluster is the right size, put it in finalClusters
            finalClusters.push_back(clusters[i]);
        }
    }

    // now we have the thresholded image we can redraw image to show clustering
    unsigned char *imgPointer;

    for (int i = 0; i < outImage.rows; ++i)
    {
        imgPointer = outImage.ptr<unsigned char>(i);
        for (int j = 0; j < outImage.cols; ++j)
        {
            if (pointFlags[i][j] == noise)
                imgPointer[j] = 0;
            else if (pointFlags[i][j] % 2 == perimeter % 2)
                imgPointer[j] = 255;
            else
                imgPointer[j] = 128;
        }
    }

    // return the vector of clusters
    return clusters;
}

} // namespace rrec