
#ifndef PROCESSIMAGES_H
#define PROCESSIMAGES_H
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include <boost/filesystem.hpp>

static const int IMWIDTH=640;
static const int IMHEIGHT=480;
static const int DEFAULT_EXPOSURE=50;
static const float SPHERE_RAD_CM = 1.0;
static const float CIRC_DIA_CM = 0.5;
static const float TOP_RAD_CM = 0.5;
static const float PROBE_HT_CM = 3.0;

struct imageInfo {
  float HSVTop[6];
  float HSVBottom[6];
  float canVals[2];
  int exposure;
  float pxPerCm;
};
  
cv::VideoCapture initCamera(int);

imageInfo calibrateIm(cv::VideoCapture, int);
void nothing(int,void*);
int largestContourIdx(std::vector<std::vector<cv::Point>>);
float getPxPerCm(cv::Mat);
void getTissueLoc(float*, float, float, float, float*);
void savePic(cv::VideoCapture, boost::filesystem::path, std::string, int, int*);
#endif
