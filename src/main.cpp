#include <iostream>

#include "screencap.hpp"
#include "processing.hpp"

int main()
{
  cv::VideoCapture cap("../video/_dvcr006.mpg");
  // cap is the object of class video capture that tries to capture Bumpy.mp4
  if (!cap.isOpened()) // isOpened() returns true if capturing has been initialized.
  {
    std::cout << "Cannot open the video file. \n";
    return -1;
  }

  cv::namedWindow("Brightness scale", CV_WINDOW_AUTOSIZE);
  cv::namedWindow("Processed video", CV_WINDOW_AUTOSIZE);

  int secondsSkipped = 355;
  // skip the first secondsSkipped seconds of the movie
  // multiplication by 1000 to deal with seconds->milliseconds
  cap.set(CV_CAP_PROP_POS_MSEC, secondsSkipped * 1000);

  while (1)
  {
    cv::Mat frame;

    // read() decodes and captures the next frame.
    if (!cap.read(frame)) // if not success, break loop
    {
      std::cout << "\n Cannot read the video file. \n";
      exit(-1);
    }

    // now it is time to process the frame:
    // make sure the video is grayscale
    cv::cvtColor(frame, frame, CV_BGR2GRAY);

    // equalize the colour histogram (this should make movies from different
    // sources look reasonably similar, making the code more portable)
    // additionally contrast is always increased wich is useful for processing
    cv::equalizeHist(frame, frame);

    // we want the gaussian blur value at each pixel to give us a gauge of
    // relative brightness in that region of the image
    int xScale = frame.cols / 5;
    int yScale = frame.rows / 5;

    // xScale and yScale need to be odd numbers so that GaussianBlur works
    if ((xScale % 2) == 0)
      xScale += 1;
    if ((yScale % 2) == 0)
      yScale += 1;

    cv::Mat brightnessScale;
    cv::GaussianBlur(frame, brightnessScale, cv::Size(xScale, yScale), 0);

    cv::Mat localBrightness;
    cv::GaussianBlur(frame, localBrightness, cv::Size(7, 7), 0);

    cv::imshow("Processed video", localBrightness);
    cv::imshow("Brightness scale", brightnessScale);

    rrec::cluster(localBrightness, brightnessScale);

    if (cv::waitKey(30) != -1) // Wait for any key press
    {
      break;
    }
  }

  return 0;
}