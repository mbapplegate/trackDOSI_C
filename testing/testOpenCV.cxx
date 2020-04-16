#include "processImages.h"
#include <iostream>
#include <stdio.h>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <boost/filesystem.hpp>

int main(void) {

  cv::VideoCapture cap = initCamera(0);

  imageInfo ret = calibrateIm(cap,1);

  boost::filesystem::path x = "/home/matthew/Desktop";
  std::string sampName = "test";

  int canVals[2] = {40,70};

  savePic(cap,x,sampName, 0, canVals);
  savePic(cap, x, sampName, 1, canVals);

  return 0;
}
