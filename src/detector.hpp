#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
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

    char pixel_from_intensity(std::vector<int> intensity, int num_pixels);

  public:
    bool is_open;       // true if image was loaded properly into RAM
    bool is_background; // true if calculate_background has been called properly
    bool is_signal;     // true if calculate_signal has been called properly
    bool is_clustered;  // true if calculate_significance has been called
    void err_not_open();

    cv::Mat get_image_main();
    cv::Mat get_image_L();
    cv::Mat get_image_d();
    cv::Mat get_image_clustered();
    void set_image_main(cv::Mat image);

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

    void equalize(); // calls an ordinary histogram equalization routine

    // an adaptive histogram equalization algorithm
    void adaptive_hist_eq(int length);

    // creates image_L, which is an image representing weighted mean pixel vals
    void calculate_background(int L);

    // creates image_d, which is an image representing signal at each pixel
    void calculate_signal(int d);

    // creates image_clustered, which for now is a thresholded image
    void calculate_significance(double sigma);

    // clusters image_clustered if available, else it clusters image_main
    void cluster();
    void print_clusters();
};
} // namespace rrec