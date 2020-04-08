//Testing the readASC Data class and functions

#include <iostream>
#include "ASCData.h"
#include <string>
#include <boost/filesystem.hpp>


int main(void){

  std::string fname = "exampleData/inc2xa-000.asc";

  ASCData a1 = getASCData(fname);
  std::cout << a1.amp.size() << std::endl;
  /*
  for (int i = 0; i<a1.numFreqs; i++) {
    for (int j = 0; j<a1.nDiodes; j++) {
      std::cout << a1.amp[i*a1.nDiodes + j] << ", ";
    }
    std::cout << std::endl;
  }
  */
  a1 = stripNaNFreqs(a1);
  
  for (int i = 0; i<a1.numFreqs; i++) {
    std::cout << a1.freqs[i] << ": ";
    for (int j = 0; j<a1.nDiodes; j++) {
      std::cout << a1.amp[i*a1.nDiodes + j] << ", ";
    }
    std::cout << std::endl;
  }
  
  std::cout << a1.numFreqs << std::endl;

  std::vector<boost::filesystem::path> files;
  boost::filesystem::path root = "exampleData";
  std::string ext = "Acrin9-";
  //getFiles(root,ext, files);
  ASCData avg = averageASCData(root,ext);

   for (int i = 0; i<avg.numFreqs; i++) {
    std::cout << avg.freqs[i] << ": ";
    for (int j = 0; j<avg.nDiodes; j++) {
      std::cout << avg.phase[i*avg.nDiodes + j] << ", ";
    }
    std::cout << std::endl;
  }
 
  return 0;
}
  
