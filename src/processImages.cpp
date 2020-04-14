#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "processImages.h"
#include <iostream>

cv::VideoCapture initCamera(int device) {

  int apiID = cv::CAP_ANY;

  cv::VideoCapture cap;
  cap.open(device+apiID);
  
  if (!cap.isOpened()) {
    std::cerr << "Can't open camera\n";
    //return -1;
  }

  cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
  cap.set(cv::CAP_PROP_FRAME_WIDTH, IMWIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, IMHEIGHT);
  cap.set(cv::CAP_PROP_FPS,30);
  cap.set(cv::CAP_PROP_EXPOSURE, DEFAULT_EXPOSURE);
  return cap;
}

imageInfo calibrateIm(cv::VideoCapture cap, int breastFlag) {
  imageInfo ret;
  cv::namedWindow("controlTop");
  cv::namedWindow("controlBottom");
  cv::namedWindow("tracked");
  int hue_low_top = 99;
  int hue_high_top = 137;
  int sat_low_top = 106;
  int sat_high_top = 255;
  int val_low_top = 30;
  int val_high_top = 255;

  int hue_low_bot = 41;
  int hue_high_bot = 101;
  int sat_low_bot = 63;
  int sat_high_bot = 255;
  int val_low_bot = 20;
  int val_high_bot = 255;

  int cannyHigh = 70;
  int cannyLow = 40;
  int quitFlag = 1;
  int exposure = DEFAULT_EXPOSURE;
  int lastExposure = exposure;

  
  cv::createTrackbar("Canny Low", "tracked", &cannyLow, 255,nothing);
  cv::createTrackbar("Canny High", "tracked", &cannyHigh, 255,nothing);
  cv::createTrackbar("Exposure", "tracked", &exposure,3000,nothing);
  
  cv::createTrackbar("Hue Low", "controlTop", &hue_low_top, 179, nothing);
  cv::createTrackbar("Hue High", "controlTop", &hue_high_top,179,nothing);
  cv::createTrackbar("Sat Low", "controlTop", &sat_low_top, 255, nothing);
  cv::createTrackbar("Sat High", "controlTop", &sat_high_top,255,nothing);
  cv::createTrackbar("Val Low", "controlTop", &val_low_top, 255, nothing);
  cv::createTrackbar("Val High", "controlTop", &val_high_top,255,nothing);

  cv::createTrackbar("Hue Low", "controlBottom", &hue_low_bot, 179, nothing);
  cv::createTrackbar("Hue High", "controlBottom", &hue_high_bot,179,nothing);
  cv::createTrackbar("Sat Low", "controlBottom", &sat_low_bot, 255, nothing);
  cv::createTrackbar("Sat High", "controlBottom", &sat_high_bot,255,nothing);
  cv::createTrackbar("Val Low", "controlBottom", &val_low_bot, 255, nothing);
  cv::createTrackbar("Val High", "controlBottom", &val_high_bot,255,nothing);

 
  int pxPerCm = 1;
  //Raw image from the camera
  cv::Mat image = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT),CV_8UC3);
  //Totally blue image
  cv::Mat blueIm = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT),CV_8UC3);
  //Totally red image
  cv::Mat redIm = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT),CV_8UC3);
  //Camera image converted to HSV space
  cv::Mat hsv = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);
  //Blue Image only on the Lower half of the sphere 
  cv::Mat blueLower = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);
  //Red image only on the upper half of the sphere
  cv::Mat redUpper= cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);
  //1-bit Image mask for the upper part of the sphere
  cv::Mat maskUpper = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8U);
  //1-bit Image mask for the lower part of the sphere
  cv::Mat maskLower = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8U);
  //1-bit Image mask containing the inverse of maskUpper AND maskLower
  cv::Mat maskBoth = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT),CV_8U);
  //Color Image with hole defined by maskBoth
  cv::Mat imMask = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);
  //Intermediate images
  cv::Mat upperMask = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);
  cv::Mat lowerMask = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);
  cv::Mat outIm =  cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT), CV_8UC3);

  cv::Mat pHolder = cv::Mat::zeros(cv::Size(640,5), CV_8U);
  blueIm.setTo(cv::Scalar(255,0,0));
  redIm.setTo(cv::Scalar(0,0,255));
  
  while (quitFlag) {
    ret.HSVTop[0] =hue_low_top;
    ret.HSVTop[1]=sat_low_top;
    ret.HSVTop[2]=val_low_top;
    ret.HSVTop[3] = hue_high_top;
    ret.HSVTop[4] = sat_high_top;
    ret.HSVTop[5] =val_high_top;
    ret.HSVBottom[0] =hue_low_bot;
    ret.HSVBottom[1] = sat_low_bot;
    ret.HSVBottom[2] = val_low_bot;
    ret.HSVBottom[3] = hue_high_bot;
    ret.HSVBottom[4] = sat_high_bot;
    ret.HSVBottom[5] = val_high_bot;
    ret.canVals[0] =cannyLow;
    ret.canVals[1] = cannyHigh;

    if (exposure != lastExposure) {
      cap.set(cv::CAP_PROP_EXPOSURE, exposure);
    }

    lastExposure = exposure;
    float sphereRadiusPx = SPHERE_RAD_CM * pxPerCm;

    cap.read(image);
    //Set HSV image
    cv::cvtColor(image,hsv,cv::COLOR_BGR2HSV);
    //Define Lower and upper masks
    cv::inRange(hsv,ret.HSVTop[0], ret.HSVTop[3],maskUpper);
    cv::inRange(hsv,ret.HSVBottom[0], ret.HSVBottom[3], maskLower);
    //Combine both masks and invert
    cv::bitwise_or(maskLower, maskUpper, maskBoth);
    cv::bitwise_not(maskBoth,maskBoth);
    //std::cout << "Print maskLower rows: " << maskLower.depth() << std::endl;
    //std::cout << "Print image rows: " << blueIm.cols << std::endl;
    //Define blue and red images
    cv::bitwise_and(blueIm, blueIm, blueLower,maskLower);
    cv::bitwise_and(redIm,redIm, redUpper, maskUpper);

    
    if (breastFlag) {
      std::cout << "Not implemented yet" << std::endl;
    }
    else {
      //Set the masked image
      cv::bitwise_and(image,image, imMask,maskBoth);
    }
    //    std::cout << "Exposure: " << exposure << std::endl;
    cv::add(imMask, blueLower, upperMask);
    cv::add(imMask, redUpper, lowerMask);
    cv::add(upperMask,lowerMask,outIm);

    cv::imshow("tracked", maskUpper);
    cv::imshow("controlTop", pHolder);
    cv::imshow("controlBottom", pHolder);
    int key = cv::waitKey(1);

    if (key == 'q') {
      quitFlag = 0;
    }
  }
  cv::destroyAllWindows();
  return ret;
}

void nothing(int a, void* b){
  return;
}
