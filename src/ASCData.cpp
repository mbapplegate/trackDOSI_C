//Class that defines data coming from outside ASC files

#include "ASCData.h"
#include "utilities.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <dirent.h>
#include <sys/types.h>

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>
//This constructor parses the ASC file in question and fills in the appropriate
//fields
ASCData getASCData(std::string fname)
{
  ASCData thisDat;
  //Initialize these to zero in case they aren't included
  thisDat.posX = 0;
  thisDat.posY = 0;
  thisDat.Theta = 0;
  thisDat.Phi = 0;
  //std::cout << fname << std::endl;

  //Open the file
  std::ifstream f (fname);
  std::string line;
  std::string delim = ",";
  //Check to make sure it opened properly
  if (f.is_open()){
    //std::cout << "File open" << std::endl;
    getline(f,line); //Read a line
    while ( line.find("Frequency") == std::string::npos) {
      // std::cout << line << std::endl;
      //Look for This string
      if (line.find("Source-Detector") != std::string::npos) {
	//Save to the right variable
	thisDat.SDSep = atoi(line.substr(22, line.length()-1).c_str());
	//std::cout << SDSep << std::endl;
      }
      else if (line.find("Patient ID:") != std::string::npos) {
	thisDat.patientID = line.substr(12, line.length()-1);
	//std::cout << patientID << std::endl;
      }
      //Number of frequencies
      else if (line.find("Number of Points:") != std::string::npos) {
	thisDat.numFreqs = atoi(line.substr(18, line.length()-1).c_str());
      }
      //Wavelength is a little more involved since it's comma separated list
      else if (line.find("Laser names:") != std::string::npos) {
	//Get the list of numbers
	std::string nameStr = line.substr(13, line.length()-1);
	while (nameStr.length() > 1) {
	  //Get the first element of the list
	  std::string token = nameStr.substr(0,nameStr.find(delim));
	  //Erase the first element of the list plus the delimiter
	  nameStr.erase(0,nameStr.find(delim)+1);
	  //std::cout << token << std::endl;
	  //Parse the list
	  thisDat.wavelengths.push_back(atoi(token.c_str()));
	}
	thisDat.nDiodes = thisDat.wavelengths.size();
      }
      //Get the XY Position and the Angle (if available)
      else if (line.find("Pos X") != std::string::npos) {
	int i = 0;
	int stop = 0;
	while (stop == 0) {
	  std::string token1 = line.substr(0, line.find(','));
	  std::string token2 = token1.substr(line.find(": ")+2, token1.length());
	  //std::cout << token2 << std::endl;
	  if (i == 0) { thisDat.posX = atof(token2.c_str()); }
	  else if (i == 1) { thisDat.posY = atof(token2.c_str()); }
	  else if (i == 2) { thisDat.Theta = atof(token2.c_str()); }
	  else if (i == 3) { thisDat.Phi = atof(token2.c_str()); }
	  if (line.find(',') == std::string::npos) {
	    stop = 1;
	  }
	  line.erase(0, line.find(',')+2);
	  //std::cout << line << std::endl;
	  i++;
	}
      }
      getline(f,line);
    }
 
    thisDat.phase.reserve(thisDat.numFreqs*thisDat.nDiodes);
    thisDat.amp.reserve(thisDat.numFreqs*thisDat.nDiodes);
    thisDat.reim.reserve(thisDat.numFreqs*thisDat.nDiodes);
    //thisDat.calReim.reserve(thisDat.numFreqs*thisDat.nDiodes);
    int lineNum = 0;
    float onePhase=0;
    float oneAmp=0;
    std::complex<float> oneReIm;
    while (true) {
      getline(f,line); //Get next data line
      if (f.eof()) { break;}
      for (int i = 0; i < 2*thisDat.nDiodes + 1; i ++) {
	std::string token = line.substr(0, line.find('\t'));
	if (i == 0) {
	  //int v = atoi(token.c_str());
	  thisDat.freqs.push_back(atof(token.c_str()));
	  //std::cout << v << std::endl;
	}
	else if (i % 2 == 0) {
	  oneAmp = atof(token.c_str());
	  thisDat.amp.push_back( oneAmp);
	  thisDat.damp.push_back(0.03);
	  //std::cout << lineNum*nDiodes + (i/2) << std::endl;

	  oneReIm = {oneAmp * cos(onePhase), oneAmp * sin(onePhase)}; 
	  thisDat.reim.push_back(oneReIm);
	}
	else {
	  onePhase = atof(token.c_str())*M_PI/180;
	  thisDat.phase.push_back( onePhase );
	  thisDat.dphase.push_back(0.3*M_PI/180);
	}
	line.erase(0,line.find('\t')+1);
      }
      lineNum++;
      //std::cout << line << std::endl;
    }
  }
  else {
    std::cerr << "Couldn't open file" <<std::endl;
  }
  
  f.close();

  return thisDat;
}

ASCData stripNaNFreqs(ASCData dat) {
  std::vector<int> nanRows;
  for (int i = 0; i < dat.numFreqs; i++) {
    std::vector<float> thisAmp(dat.amp.begin()+(i*dat.nDiodes), dat.amp.begin()+(i+1)*dat.nDiodes);
    //std::cout << thisAmp[0] << std::endl;
    for (int j = 0; j< dat.nDiodes; j++) {
      // std::cout << thisAmp[j] << std::endl;
      if (std::isnan(thisAmp[j])) {
	nanRows.push_back(i);
	//std::cout << "Has Nans" <<std::endl;
	break;
      }
    }
    //if (hasNans == 1) {
    // 
    // }
   //std::cout << thisAmp[0] << std::endl;
  }
  int numGone = 0;
  for (size_t i = 0; i<nanRows.size(); i++) {
    dat.amp.erase(dat.amp.begin()+((nanRows[i]-numGone)*dat.nDiodes), dat.amp.begin()+(nanRows[i]+1-numGone)*dat.nDiodes);
    dat.phase.erase(dat.phase.begin()+((nanRows[i]-numGone)*dat.nDiodes), dat.phase.begin()+(nanRows[i]+1-numGone)*dat.nDiodes);
    dat.reim.erase(dat.reim.begin() + ((nanRows[i]-numGone)*dat.nDiodes), dat.reim.begin()+(nanRows[i]+1-numGone)*dat.nDiodes);
    dat.freqs.erase(dat.freqs.begin()+nanRows[i]-numGone);
    numGone++;
  }
  dat.numFreqs = dat.freqs.size();
  return dat;
}



//Function to average a bunch of raw data from ASC files. Used to get IRF

ASCData averageASCData(boost::filesystem::path dName, std::string fStr) {

  std::vector<boost::filesystem::path> fList;
  //boost::filesystem::path root = dName;
 
  getFiles(dName, fStr, fList);
  std::cout << fList[0].native() << std::endl;
  boost::filesystem::path fullFile;
  fullFile = dName / fList[0];
  std::cout << fullFile.native() << std::endl;
  ASCData *a1 = new ASCData[fList.size()];
  ASCData ret = getASCData(fullFile.native());
  ret = stripNaNFreqs(ret);
  std::vector<float> runSumAmp(ret.amp.size(), 0.0);
  std::vector<float> runSumPhase(ret.phase.size(), 0.0);
  std::vector<std::complex<float>> runSumReim(ret.reim.size());
  //  std::cout << runSumAmp.size() << std::endl;
  for (size_t i = 0; i< fList.size(); i++) {
    fullFile = dName / fList[i];
    ASCData thisDat = getASCData(fullFile.native());
    
    thisDat = stripNaNFreqs(thisDat);
    runSumAmp = sumVecs(thisDat.amp, runSumAmp);
    runSumPhase = sumVecs(thisDat.phase, runSumPhase);
    runSumReim = sumVecs(thisDat.reim, runSumReim);
    //std::cout << "First reim: " << runSumReim[0] << std::endl;
  }
  ret.amp=   divVecs(runSumAmp,(float)fList.size());
  ret.phase= divVecs(runSumPhase,(float)fList.size());
  std::vector<float> runSumAmp2(ret.amp.size(), 0.0);
  std::vector<float> runSumPhase2(ret.phase.size(), 0.0);
  std::vector<float> diffVecAmp(ret.amp.size(), 0.0);
  std::vector<float> diffVecPhase(ret.phase.size(), 0.0);
  
  for (size_t i = 0; i<fList.size(); i++) {
    ASCData thisDat = getASCData(fullFile.native());
    thisDat = stripNaNFreqs(thisDat);
    diffVecAmp = diffVecs(thisDat.amp,ret.amp);
    diffVecPhase = diffVecs(thisDat.phase,ret.phase);
    diffVecAmp = squareVecs(diffVecAmp);
    diffVecPhase = squareVecs(diffVecPhase);
    runSumAmp2=sumVecs(diffVecAmp, runSumAmp2);
    runSumPhase2 = sumVecs(diffVecPhase,runSumPhase2);

  }

  std::vector<float> stdAmp = divVecs(runSumAmp2,(fList.size()-1));
  std::vector<float> stdPhase = divVecs(runSumPhase2, (fList.size()-1));

  ret.damp = sqrtVecs(stdAmp);
  ret.dphase = sqrtVecs(stdPhase);
  //ASCData a2 = getASCData(fList[0].native());
  //ASCData a3 = getASCData(fList[1].native());
  //ret.amp.clear();
  //ret.phase.clear();
  
  //ret.reim = runSumReim;//divVecs(runSumReim, 10.0);
  //std::vector<float> a4 = sumVecs(a2.amp,a3.amp);
  ret.reim.clear();
  //std::vector<std::complex<float>> result;
  //std::complex<float> d((float)fList.size(),0);
  for (size_t i = 0; i<ret.amp.size(); i++) {
    ret.reim.push_back(runSumReim[i]/(float)fList.size());
  }
  //std::cout << runSumReim[0] << "/" << "30.0" << "=" << runSumReim[0]/(float)30.0 << std::endl;
  
  delete[] a1;
  return ret;
}

std::vector<std::complex<float>> getOneWavelengthComplex(std::vector<std::complex<float>> dat, int lambdaIdx, int numDiodes, int numFreqs) {
  std::vector<std::complex<float>> ret;
  ret.reserve(numFreqs);
  for (int i = 0; i<numFreqs; i++) {
    ret.push_back(dat[i*numDiodes + lambdaIdx]);
  }
  return ret;
}

std::vector<std::complex<float>> getOneWavelengthComplex(ASCData dat, int lambdaIdx) {

  std::vector<std::complex<float>> ret;
  ret.reserve(dat.numFreqs);
  for (int i = 0; i<dat.numFreqs; i++) {
    ret.push_back(dat.reim[i*dat.nDiodes + lambdaIdx]);
  }
  return ret;
}

//Return the amplitude and phase for one wavelength
void getOneWavelengthAmpPhase(ASCData dat,int lambdaIdx, std::vector<float>* amp, std::vector<float>* phase) {

  amp->reserve(dat.numFreqs);
  phase->reserve(dat.numFreqs);
  for (size_t i = 0; i<dat.freqs.size(); i++) {
    amp->push_back(dat.amp[i*dat.nDiodes + lambdaIdx]);
    phase->push_back(dat.phase[i*dat.nDiodes + lambdaIdx]);
  }
}


//Return the amplitude and phase for one wavelength
std::vector<float> getOneWavelengthAmp(ASCData dat,int lambdaIdx) {
  std::vector<float> amp;
  amp.reserve(dat.freqs.size());  
  for (size_t i = 0; i<dat.freqs.size(); i++) {
    std::cout << dat.amp.size() << std::endl;
    std::cout << i*dat.nDiodes+lambdaIdx << std::endl;
    amp.push_back(dat.amp[i*dat.nDiodes + lambdaIdx]);
  }
  return amp;
}


//Return the amplitude and phase for one wavelength
std::vector<float> getOneWavelengthPhase(ASCData dat,int lambdaIdx) {
  std::vector<float> phase;
  phase.reserve(dat.freqs.size());  
  for (size_t i = 0; i<dat.freqs.size(); i++) {
    
    phase.push_back(dat.amp[i*dat.nDiodes + lambdaIdx]);
  }
  return phase;
}



// return the filenames of all files that have the string fname
// in the specified directory
void getFiles(boost::filesystem::path root, std::string fname, std::vector<boost::filesystem::path>& ret)
{
  if(!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)){
    return;
  }
  boost::filesystem::directory_iterator it(root);
  boost::filesystem::directory_iterator endit;
  std::string testName;
  while(it != endit) {
    if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ".asc") {
      testName = it->path().filename().native();
      if (testName.find(fname) != std::string::npos) {
	ret.push_back(it->path().filename());
      }
    }
    ++it;
  }
}
