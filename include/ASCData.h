//Header file for ASCData defines the class that will be used to parse data from ASC Files

#ifndef ASCDATA_H
#define ASCDATA_H
#include <vector>
#include <string>
#include <complex>
#include <boost/filesystem.hpp>
//#include <iostream>
//#include <stdio.h>
struct ASCData {
  std::vector<float> freqs;
  int numFreqs;
  std::vector<float> phase;
  std::vector<float> amp;
  std::vector<std::complex<float>> reim;
  std::vector<int> wavelengths;
  std::string patientID;
  int nDiodes;
  int SDSep;
  float posX;
  float posY;
  float Theta;
  float Phi;

};


ASCData getASCData(std::string);
ASCData stripNaNFreqs(ASCData);
void getFiles(boost::filesystem::path, std::string, std::vector<boost::filesystem::path>&);
ASCData averageASCData(boost::filesystem::path,std::string);
std::vector<float> getOneWavelengthAmp(ASCData, int);
std::vector<float> getOneWavelengthPhase(ASCData, int);
void getOneWavelengthAmpPhase(ASCData, int,std::vector<float>*, std::vector<float>*);

std::vector<std::complex<float>> getOneWavelengthComplex(std::vector<std::complex<float>>, int,int,int);

std::vector<std::complex<float>> getOneWavelengthComplex(ASCData, int);
#endif
