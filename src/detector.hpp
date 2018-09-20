#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "dbscan.hpp"
#include "cluster.hpp"

namespace rrec
{
class Detector
{
  private:
    cv::Mat image_main;
    cv::Mat image_L;
    cv::Mat image_d;
    cv::Mat image_clustered;

    std::vector<rrec::Cluster> clusters;

    std::string path;

    float pic_cutoff; // .pic max threshold, defaults to 900 (see constructors)

  public:
    bool is_open; // true if image was loaded properly into RAM

    void load_vector(std::vector<char> image); // not implemented
    void load_image(std::string path);
    void load_image();
    void load_pic(std::string path, float cutoff, int rows, int cols);
    void load_pic(std::string path, int rows, int cols);
    void load_pic(float cutoff, int rows, int cols);
    void load_pic(int rows, int cols);

    Detector(std::string path, int rows, int cols);
    Detector(std::string path);
    Detector();

    void equalize();

    void calculate_background(int L);

    void calculate_signal(int d);

    void calculate_significance(double sigma);

    void cluster();
};
} // namespace rrec