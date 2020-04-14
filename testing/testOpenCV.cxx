#include "processImages.h"
#include <iostream>
#include <stdio.h>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"

int main(void) {

  cv::VideoCapture cap = initCamera(0);

  imageInfo ret = calibrateIm(cap,0);
  return 0;
}
