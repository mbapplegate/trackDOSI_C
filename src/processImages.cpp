#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "processImages.h"
#include <iostream>
#include <cmath>

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

  double ctrUpper[2] = {0,0};
  double ctrLower[2] = {0,0};
  double tissueLoc[2] = {0,0};
  double phi = 0;
  double theta = 0;
  while (quitFlag) {
    blueLower = cv::Mat::zeros(blueLower.size(), blueLower.type());
    redUpper = cv::Mat::zeros(redUpper.size(), redUpper.type());
    //maskUpper = cv::Mat::zeros(maskUpper.size(), maskUpper.type());
    //maskLower = cv::Mat::zeros(maskLower.size(), maskLower.type());
    outIm = cv::Mat::zeros(outIm.size(), outIm.type());
    //upperMask = cv::Mat::zeros(upperMask.size(), upperMask.type());
    //lowerMask = cv::Mat::zeros(lowerMask.size(), lowerMask.type());
    //maskBoth = cv::Mat::zeros(maskBoth.size(), maskBoth.type());
    imMask = cv::Mat::zeros(imMask.size(), imMask.type());
    
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
      std::cout << "Exposure set to: " << exposure <<std::endl;
    }

    lastExposure = exposure;
    //float sphereRadiusPx = SPHERE_RAD_CM * pxPerCm;

    cap.read(image);
    //Set HSV image
    cv::cvtColor(image,hsv,cv::COLOR_BGR2HSV);
    //Define Lower and upper masks
    cv::inRange(hsv,cv::Scalar(ret.HSVTop[0],ret.HSVTop[1],ret.HSVTop[2]), cv::Scalar(ret.HSVTop[3],ret.HSVTop[4],ret.HSVTop[5]),maskUpper);
    cv::inRange(hsv,cv::Scalar(ret.HSVBottom[0],ret.HSVBottom[1],ret.HSVBottom[2]), cv::Scalar(ret.HSVBottom[3],ret.HSVBottom[4],ret.HSVBottom[5]), maskLower);
    //Combine both masks and invert
    cv::bitwise_or(maskLower, maskUpper, maskBoth);
    cv::bitwise_not(maskBoth,maskBoth);
    //std::cout << "Print maskLower rows: " << maskLower.depth() << std::endl;
    //std::cout << "Print image rows: " << blueIm.cols << std::endl;
    //Define blue and red images
    cv::bitwise_and(blueIm, blueIm, blueLower,maskLower);
    cv::bitwise_and(redIm,redIm, redUpper, maskUpper);

    int angleFound = getAngle(maskUpper, maskLower,pxPerCm,0.0,&theta,&phi,&ctrLower[0],&ctrUpper[0]);
    
    if (breastFlag) {
      cv::Mat grayIm = cv::Mat::zeros(image.size(),CV_8U);
      cv::Mat edgeIm = cv::Mat::zeros(image.size(),CV_8U);
      cv::Mat colorEdge = cv::Mat::zeros(image.size(),image.type());
      cv::cvtColor(image, grayIm, cv::COLOR_BGR2GRAY);
      cv::Canny(grayIm, edgeIm, cannyLow, cannyHigh);
      cv::cvtColor(edgeIm,colorEdge,cv::COLOR_GRAY2BGR);
      cv::bitwise_and(colorEdge, image, imMask,maskBoth);
      
    }
    else {
      //Set the masked image
      cv::bitwise_and(image,image, imMask,maskBoth);
    }
    //    std::cout << "Exposure: " << exposure << std::endl;
    cv::add(imMask, blueLower, upperMask);
    cv::add(imMask, redUpper, lowerMask);
    cv::add(upperMask,lowerMask,outIm);

    if (angleFound >= 0) {
      cv::circle(outIm, cv::Point(ctrUpper[0],ctrUpper[1]), 5, cv::Scalar(255,0,0),-1);
      cv::circle(outIm, cv::Point(ctrLower[0], ctrLower[1]), 5, cv::Scalar(0,0,255), -1);
      getTissueLoc(&ctrLower[0], pxPerCm, theta, phi,&tissueLoc[0]);
    }
    cv::imshow("tracked", outIm);
    cv::imshow("controlTop", pHolder);
    cv::imshow("controlBottom", pHolder);
    int key = cv::waitKey(1);
    //std::cout << "Top Center: " << ctrUpper[0] << ", " << ctrUpper[1] << std::endl;
    if (key == 'q') {
      quitFlag = 0;
    }
    else if (key == 's') {
      pxPerCm = getPxPerCm(maskLower);
      std::cout << "pxPerCm: " << pxPerCm << std::endl;
    }
    else if (key == 'p') {
      std::cout << "X: " << tissueLoc[0] << ", Y: " << tissueLoc[1] <<
	", Theta: " << theta << ", Phi: " << phi << std::endl;
    }
  }
  cv::destroyAllWindows();
  return ret;
}

int largestContourIdx(std::vector<std::vector<cv::Point>> contours) {
  int largest_contour_index = -1;
  float largest_area = 0;
  for (size_t i = 0; i<contours.size(); i++) {
    float a = cv::contourArea(contours[i], false);
    if (a > largest_area) {
      largest_area = a;
      largest_contour_index = i;
    }
  }
  return largest_contour_index;
}
float getPxPerCm(cv::Mat maskLower) {
  std::vector<cv::Vec4i> hierarchy;
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(maskLower.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  int largeIdx = largestContourIdx(contours);
  cv::Point2f ctr;
  float rad;
  cv::minEnclosingCircle((cv::Mat)contours[largeIdx], ctr, rad);

  float pxPerCm = rad/(2*SPHERE_RAD_CM);
  return pxPerCm;

}

void getTissueLoc(float *ctrLower, float probeHtPx, float theta, float phi, float* tissueLoc) {
  tissueLoc[0] = ctrLower[0] - probeHtPx*sin(phi)*cos(theta);
  tissueLoc[1] = ctrLower[1] - probeHtPx*sin(phi)*sin(theta);
}

void savePic(cv::VideoCapture cap, boost::filesystem::path datDir, std::string sampName, int breastFlag, int* canVals) {
    cv::Mat image = cv::Mat::zeros(cv::Size(IMWIDTH,IMHEIGHT),CV_8UC3);
    boost::filesystem::path fullFile = datDir / (sampName + "_img.png");
    cap.read(image);
    if (breastFlag == 1) {
      cv::Mat greyIm = cv::Mat::zeros(image.size(), CV_8U);
      cv::Mat edgeIm = cv::Mat::zeros(image.size(), CV_8U);
      cv::cvtColor(image,greyIm,cv::COLOR_BGR2GRAY);
      cv::Canny(greyIm, edgeIm, canVals[0], canVals[1]);
      cv::imwrite(fullFile.string(),edgeIm);
    }
    else {
      cv::imwrite(fullFile.string(), image);
    }
}

int getAngle(cv::Mat maskUpper, cv::Mat maskLower, float pxPerCm, float phi0, double* theta, double* phi, double* ctrLower, double* ctrUpper){
  std::vector<cv::Vec4i> hierarchy;
  std::vector<std::vector<cv::Point>> contoursLower;
  std::vector<std::vector<cv::Point>> contoursUpper;

  double cXupper = 0;
  double cYupper = 0;
  double cXlower = 0;
  double cYlower = 0;
  cv::Moments Mlower, Mupper;
  int largestIdxUpper = -1;
  int largestIdxLower = -1;
  cv::findContours(maskLower.clone(), contoursLower, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  largestIdxLower = largestContourIdx(contoursLower);
  
  cv::findContours(maskUpper.clone(), contoursUpper, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  largestIdxUpper = largestContourIdx(contoursUpper);

  if (largestIdxLower >= 0) {
    Mlower = cv::moments(contoursLower[largestIdxLower], false);
  }
  else {return -1;}
  if (largestIdxUpper >= 0) {
    Mupper = cv::moments(contoursUpper[largestIdxUpper], false);
  }
  else{return -1;}

  if (Mlower.m00 != 0) {
    cXlower = Mlower.m10/Mlower.m00;
    cYlower = Mlower.m01/Mlower.m00;
  }
  else{return -1;}
  if (Mupper.m00 != 0) {
    cXupper = Mupper.m10/Mupper.m00;
    cYupper = Mupper.m01/Mupper.m00;
    
  }
  else{return -1;}
  double th = atan2(cYupper - cYlower, cXupper-cXlower);
  double d = sqrt(pow((cXupper - cXlower),2) + pow((cYupper - cYlower),2));

  double L1 = (SPHERE_RAD_CM * pxPerCm) - (TOP_RAD_CM * pxPerCm);
  //double L2 = 2*SPHERE_RAD_CM * pxPerCm;

  double Beta = acos(d/L1);
  double p = M_PI/2 - Beta - phi0;
  *phi = p;
  *theta = th;
  *ctrLower = cXlower;
  *(ctrLower+1) = cYlower;
  *ctrUpper = cXupper;
  *(ctrUpper+1) = cYupper;
  return 0;
}

void getTissueLoc(double* ctrLower, float pxPerCm, float theta, float phi, double* tissueLoc) {
  *tissueLoc = ctrLower[0] - PROBE_HT_CM*pxPerCm*sin(phi)*cos(theta);
  *(tissueLoc+1) = ctrLower[1] - PROBE_HT_CM*pxPerCm*sin(phi)*cos(theta);
}

void nothing(int a, void* b){
  return;
}
