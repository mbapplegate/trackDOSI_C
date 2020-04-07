//First pass at using openCV to do stuff in C++

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
//#include <string>
#include <iostream>
#include<stdio.h>
#include "ASCData.h"

int main(int argc, const char* argv[])
{

  bool breastFlag = 0;
  bool procFlag = 1;
  bool chromFlag = 0;
  std::string calName = "ACRIN9";
  std::string phantomFile = "ACRIN9.txt";
  ASCData t = ASCData(phantomFile);
  cv::Mat frame;

  cv::VideoCapture cap;

  int deviceID = 0;
  int apiID = cv::CAP_ANY;

  cap.open(deviceID + apiID);

  if (!cap.isOpened()) {
    std::cerr << "ERROR! Can't open camera\n";
    return -1;
  }

  std::cout << "Start grabbing" << std::endl
	    << "Press any key to stop" << std::endl;

  for (;;) {
    cap.read(frame);
    if (frame.empty()) {
      std::cerr << "ERROR! blank frame\n";
      break;
    }
    cv::imshow("Live", frame);
    if (cv::waitKey(5) >= 0)
      break;
  }
  return 0;
}
