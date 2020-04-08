//Class that defines data coming from outside ASC files

#include "ASCData.h"
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
 
    //thisDat.phase = new float[thisDat.numFreqs*thisDat.nDiodes];
    //thisDat.amp = new float[thisDat.numFreqs*thisDat.nDiodes];
    int lineNum = 0;
    while (true) {
      getline(f,line); //Get next data line
      if (f.eof()) { break;}
      for (int i = 0; i < 2*thisDat.nDiodes + 1; i ++) {
	std::string token = line.substr(0, line.find('\t'));
	if (i == 0) {
	  //int v = atoi(token.c_str());
	  thisDat.freqs.push_back(atoi(token.c_str()));
	  //std::cout << v << std::endl;
	}
	else if (i % 2 == 0) {
	  thisDat.amp.push_back( atof(token.c_str()));
	  //std::cout << lineNum*nDiodes + (i/2) << std::endl;
	}
	else {
	  thisDat.phase.push_back( atof(token.c_str())*M_PI/180.0);
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
	std::cout << "Has Nans" <<std::endl;
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
    dat.freqs.erase(dat.freqs.begin()+nanRows[i]-numGone);
    numGone++;
  }
  dat.numFreqs = dat.freqs.size();
  return dat;
}



//Function to average a bunch of raw data from ASC files. Used to get IRF
///////ONLY WORKS ON POSIX SYSTEMS//////////////////
ASCData averageASCData(std::string dName, std::string fStr) {

  std::vector<boost::filesystem::path> fList;
  boost::filesystem::path root = dName;

  getFiles(root, fStr, fList);
  ASCData a1;
  return a1;
}


// return the filenames of all files that have the specified extension
// in the specified directory and all subdirectories
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
