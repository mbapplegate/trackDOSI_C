//Header file for ASCData defines the class that will be used to parse data from ASC Files

#ifndef ASCDATA_H
#define ASCDATA_H
#include <vector>
#include <string>
//#include <iostream>
//#include <stdio.h>
struct ASCData {
  std::vector<int> freqs;
  int numFreqs;
  std::vector<float> phase;
  std::vector<float> amp;
  std::vector<int> wavelengths;
  std::string patientID;
  int nDiodes;
  int SDSep;
  float posX;
  float posY;
  float Theta;
  float Phi;

};


ASCData  getASCData(std::string);
ASCData stripNaNFreqs(ASCData);
#endif
