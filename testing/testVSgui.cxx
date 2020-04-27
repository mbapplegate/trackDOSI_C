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
#include "gnuplot_i.h"
void wait_for_key ()
{
  std::cout << std::endl << "Press a key" << std::endl;
  std::cin.clear();
  std::cin.ignore(std::cin.rdbuf()->in_avail());
  std::cin.get();
  return;
}

int main(void) {

  boost::filesystem::path dataDir = "/home/matthew/Desktop/dosiTrainingCode";
  std::string calName = "ACRIN9-017-miniLBS.asc";

  std::cout << "Reading calibration data" << std::endl;
  ASCData avg = averageASCData(dataDir,calName);
  std::vector<float> acsdsqd,phisdsqd;
  acsdsqd.reserve(avg.freqs.size());
  phisdsqd.reserve(avg.freqs.size());
  std::cout << avg.wavelengths[7] << std::endl;
  std::vector<std::complex<float>> sResp = sysResponseSweep(avg, &acsdsqd,&phisdsqd);
  std::vector<boost::filesystem::path> sampFiles;
  std::string sampName = "Acrin_";
  int flag = getFiles(dataDir,sampName, sampFiles);
  std::cout << "File error: " << flag << std::endl;
  if (flag >= 0) {
    std::cout << sampFiles[0] << std::endl;
  }
  std::sort(sampFiles.begin(),sampFiles.end());
  
  ASCData dat;
  std::vector<float> mua;
  std::vector<float> mus;
  for (size_t i = 0; i<sampFiles.size(); i++) {
    dat = averageASCData(dataDir,sampFiles[i].c_str());
    //std::cout << dat.freqs[i] << std::endl;
    std::vector<std::complex<float>> cal = calibrate(&dat.reim, &sResp);
    std::vector<float> wts = wtsAmpPhase2ReIm(acsdsqd,phisdsqd, &dat, cal);
    std::cout << "Running inversion for file: " << sampFiles[i].c_str() << std::endl;
    std::vector<float> OP = runInverseModel(dat.SDSep, dat.freqs, cal, wts, dat.nDiodes);
    
    for (int j = 0; j<dat.nDiodes; j++) {
      //std::cout << "mua: " << OP[2*j] << ", Mus: " << OP[2*j+1] << std::endl;
      mua.push_back(OP[2*j]);
      mus.push_back(OP[2*j+1]);
    }
  }

  std::cout << "Mua len: " << mua.size() << std::endl;

  std::vector<float> dGUIMua {.0187,.0165,.0095,.0085,.0075,.0071,.0169,.0150,.0077,.0082,.0073,.0072,.0187,.0177,.0079,.0086,.0070,.0065,.0187,.0179,.0077,.0086,.0071,.0071,
		       .0188,.0180,.0079,.0086,.0070,.0066,.0191,.0185,.0080,.0086,.0071,.0067,.0190,.0185,.0082,.0085,.0072,.0068,.0171,.0168,.0076,.0082,.0074,.0071};
  std::vector<float> GUIMusp{.827,.789,.699,.686,.636,.633,.865,.842,.713,.729,.732,.706,.752,.751,.602,.625,.584,.588,.766,.762,.634,.651,.613,.606,
		      .748,.737,.600,.617,.567,.571,.757,.740,.609,.635,.568,.578,.769,.751,.603,.647,.566,.573,.837,.807,.676,.688,.699,.655};

  std::vector<float> repLams(48);
  float lams[6] = {658,690,785,808,830,850};
  for (int i = 0; i<48; i++) {
    repLams.at(i) = lams[i%6];
  }
    //Plot the imaginary part
  Gnuplot g1("points");
  g1.set_style("points").plot_xy(repLams,dGUIMua);
  Gnuplot::gnuplot_cmd(g1,"ps 10");
  g1.set_style("points").plot_xy(repLams,mua);
  Gnuplot::gnuplot_cmd(g1,"pt 7");
  Gnuplot::gnuplot_cmd(g1,"ps 10");
  //g1.set_style("lines").plot_xy(x,avgReal);
  //g1.set_style("lines").plot_xy(x,sRespReal);
  g1.showonscreen();
  wait_for_key();

  return 0;

}
