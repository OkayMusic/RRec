#include <iostream>

#include "screencap.hpp"
#include "processing.hpp"

int main()
{
  // the objects we need to pass to XQueryTree
  Display *display = XOpenDisplay(nullptr);
  Window rootWindow = DefaultRootWindow(display);
  Window returnedroot;
  Window returnedparent;
  Window *children;

  unsigned int numchildren;

  // store all the children of the root window in an array of size numchildren
  // starting at memory location children
  XQueryTree(display, rootWindow, &returnedroot, &returnedparent, &children,
             &numchildren);

  // rrec::showPrintScreen();

  cv::Mat testImage = rrec::getSpecificWindow(rootWindow, display);

  char *equalized_window = "Equalized Image";
  cv::namedWindow(equalized_window, cv::WINDOW_AUTOSIZE);
  cv::imshow(equalized_window, testImage);
  cv::waitKey(0);

  return 0;
}