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

  XQueryTree(display, rootWindow, &returnedroot, &returnedparent, &children,
             &numchildren);

  cv::Mat testImage = getSpecificWindow(*(children), display);

  std::cout << testImage << std::endl;

  return 0;
}