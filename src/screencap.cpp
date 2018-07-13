#include "screencap.hpp"

namespace rrec
{
void ImageFromWindow(std::vector<uint8_t> &pixels, int &width, int &height,
                     int &bitsPerPixel, Window window, Display *display)
{

    XWindowAttributes attributes = {0};
    XGetWindowAttributes(display, window, &attributes);

    width = attributes.width;
    height = attributes.height;

    XImage *img = XGetImage(display, window, 0, 0, width, height, AllPlanes,
                            ZPixmap);

    bitsPerPixel = img->bits_per_pixel;
    pixels.resize(width * height * 4);

    memcpy(&pixels[0], img->data, pixels.size());

    XDestroyImage(img);
    XCloseDisplay(display);
}

void printSpecificScreen(Window window, Display *display)
{
    using namespace cv;
    int Width = 0;
    int Height = 0;
    int Bpp = 0;
    std::vector<std::uint8_t> Pixels;

    ImageFromWindow(Pixels, Width, Height, Bpp, window, display);

    if (Width && Height)
    {
        Mat img = Mat(Height, Width, Bpp > 24 ? CV_8UC4 : CV_8UC3, &Pixels[0]);

        namedWindow("WindowTitle", WINDOW_AUTOSIZE);
        imshow("Display window", img);

        waitKey(0);
    }
}

void showPrintScreen()
{
    using namespace cv;
    int Width = 0;
    int Height = 0;
    int Bpp = 0;
    std::vector<std::uint8_t> Pixels;

    // now get the root window
    Display *display = XOpenDisplay(nullptr);
    Window rootWindow = DefaultRootWindow(display);

    ImageFromWindow(Pixels, Width, Height, Bpp, rootWindow, display);

    if (Width && Height)
    {
        Mat img = Mat(Height, Width, Bpp > 24 ? CV_8UC4 : CV_8UC3, &Pixels[0]);

        namedWindow("WindowTitle", WINDOW_AUTOSIZE);
        imshow("Display window", img);

        waitKey(0);
    }
}

cv::Mat getSpecificWindow(Window window, Display *display)
{
    cv::Mat screenShotMat;
    using namespace cv;
    int Width = 0;
    int Height = 0;
    int Bpp = 0;
    std::vector<std::uint8_t> Pixels;

    ImageFromWindow(Pixels, Width, Height, Bpp, window, display);

    if (Width && Height)
    {
        screenShotMat = Mat(Height, Width, Bpp > 24 ? CV_8UC4 : CV_8UC3,
                            &Pixels[0]);
    }

    return screenShotMat;
}
} // namespace rrec