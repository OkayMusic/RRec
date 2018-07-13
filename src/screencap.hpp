#pragma once

#include <opencv2/opencv.hpp>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdint>
#include <vector>

void ImageFromWindow(std::vector<uint8_t> &pixels, int &width, int &height,
                     int &bitsPerPixel, Window window, Display *display);

void printSpecificScreen(Window window, Display *display);

void showPrintScreen();

cv::Mat getSpecificWindow(Window window, Display *display);
