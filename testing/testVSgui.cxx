/*testVsgui
 *
 *Script to test C++ version of the DOSI Processing code against data already processed with dosigui
 *Hopefully this will give the same (or close to the same) values for mua and musp.
 *
 *Author: Matt Applegate
 *Date created: 23 April 2020
 *In COVID quarantine
 */

#include <iostream>
#include "ASCData.h"
#include "FDPM.h"

int main(void) {

  boost::filesystem::path dataDir = "/home/matthew/Desktop/dosiTrainingCode";
  std::string calName = "ACRIN9-017-miniLBS.asc";

  std::cout << "Reading calibration data" << std::endl;
  ASCData avg = averageASCData(dataDir,calName);

  std::vector<boost::filesystem::path> sampFiles;
  std::string sampName = "Acrin_";
  int flag = getFiles(dataDir,sampName, sampFiles);
  std::cout << "File error: " << flag << std::endl;
  if (flag >= 0) {
    std::cout << sampFiles[0] << std::endl;
  }
  std::sort(sampFiles.begin(),sampFiles.end());
  std::cout <<sampFiles[0] << std::endl;
  
  return 0;

}
