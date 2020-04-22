
#include "FDPM.h"
#include "ASCData.h"
#include <iostream>
#include "interpolation.h"
#include "gnuplot_i.h"
#include "utilities.h"

void wait_for_key();
int main(void) {
  //Basic parameters
  float mua = 0.1;
  float mus = 1;
  float f = 50;
  float SDSep = 10;
  //One frequency with mua and mus separately
  std::complex<float> testOneFreq = p1SemiInf(mua,mus,f,SDSep);
  std::cout << testOneFreq << std::endl;
  
  //One frequency using alglib 1d array
  alglib::real_1d_array ops = "[.1,1]";
  std::complex<float> testOneFreq2 = p1SemiInf(ops,f,SDSep);
  std::cout << testOneFreq2 << std::endl;

  //Test Complex to amplitude and phase
  float amp;
  float phase;
  ReIm2AmpPhase(testOneFreq,&amp,&phase);
  std::cout << "[" << amp << ", " << phase << "]" << std::endl;

  //Test Amplitude and phase to complex
  std::complex<float> converted = AmpPhase2ReIm(amp,phase);
  std::cout << converted << std::endl;

  
  int wavelength = 785;
  float acrMua;
  float acrMus;

  //Test function to get OPs of ACRIN9
  getACRINops(wavelength, &acrMua, &acrMus);
  std::cout << "ACRIN mua: " << acrMua << std::endl
	    << "ACRIN mus: " << acrMus << std::endl;

  //Test function to calculate amplitude and phase system response
  //Using no noise the amplitude response should be 1, the phase should be 0
  std::complex<float> fakeExpReIm=p1SemiInf(acrMua,acrMus,f,SDSep);
  float fakeAmp;
  float fakePhase;
  ReIm2AmpPhase(fakeExpReIm,&fakeAmp,&fakePhase);
  float ampRes;
  float phaseRes;
  
  calcAmpPhaseSystemResponse(wavelength, f, SDSep, fakeAmp,fakePhase,&ampRes,&phaseRes);
  std::cout << "Amp Sys Resp: " << ampRes << std::endl
	    << "Phase Sys Resp: " << phaseRes << std::endl;
  std::complex<float> resp;
  std::complex<float> complexRes=calcReImSystemResponse(wavelength, f, SDSep, fakeExpReIm, &resp);
  
  std::cout << "fakeExpReIm: " <<  fakeExpReIm << std::endl;
  
  std::cout << "Re Sys Resp: " << complexRes.real() << std::endl
	    << "Im Sys Resp: " << complexRes.imag() << std::endl;
  
  ///////////////////////////////////////////
  //Testing for frequency sweeps
  ///////////////////////////////////////////
  //
  //Test getting theoretical values for a frequency sweep
  int numFreqs = 35;
  std::vector<float> sweepFreq;
  sweepFreq.reserve(numFreqs);
  for (size_t i = 0; i < numFreqs; i++) {
    sweepFreq.push_back(50 + i*7);
  }


  std::vector<std::complex<float>> sweepComplex = p1Sweep(acrMua,acrMus, sweepFreq, SDSep);
  std::cout << "sweep complex len: " << sweepComplex.size() << std::endl;
  std::vector<float> realVals(numFreqs);
  std::vector<float> imagVals(numFreqs);
  std::vector<float> ampVals(numFreqs);
  std::vector<float> phaseVals(numFreqs);
  float thisAmp;
  float thisPhase;
  for (size_t i=0; i<numFreqs; i++) {
    
    realVals[i]= sweepComplex[i].real();
    imagVals[i]= sweepComplex[i].imag();
    ReIm2AmpPhase(sweepComplex[i],&thisAmp,&thisPhase);
    ampVals[i] = thisAmp;
    phaseVals[i] = thisPhase;
  }

  Gnuplot g0("lines");
  g0.set_style("lines").plot_xy(sweepFreq,realVals);
  g0.set_style("lines").plot_xy(sweepFreq,imagVals);
  g0.showonscreen();


  std::vector<float> sysRespAmp;
  std::vector<float> sysRespPhase;

  sysResponseSweep(wavelength, sweepFreq, SDSep, ampVals, phaseVals, &sysRespAmp, &sysRespPhase);

  //  std::vector<std::complex<float>> x= sysResponseSweep(sweepFreq, SDSep, sweepComplex);

  std::vector<float> calAmp;
  std::vector<float> calPhase;

  calibrate(ampVals,phaseVals,sysRespAmp,sysRespPhase, &calAmp, &calPhase);

  //  for (size_t i =0; i<numFreqs; i++) {
  //  std::cout << calAmp[i] <<", (" << ampVals[i] << ", " << calPhase[i] <<", " << phaseVals[i] << ")"  <<std::endl;
  //}

  //std::vector<std::complex<float>> cal = calibrate(sweepComplex ,x);
  //for (size_t i =0; i<numFreqs; i++) {
  // std::cout << cal[i] << ", " << sweepComplex[i] << std::endl;
  //}
  //const alglib::real_1d_array ops2 = "[.015987,.7693874]";
  //float chiTest = chi(ops2, cal,SDSep,sweepFreq);
  //std::cout << chiTest << std::endl;

 
  boost::filesystem::path dStr = "exampleData";
  std::string fname = "Acrin9";

  ASCData avg = averageASCData(dStr,fname);
  std::cout << "average data len: " << avg.reim.size() << std::endl;
  
  ASCData expData = getASCData("exampleData/Acrin9-000.asc");
  expData = stripNaNFreqs(expData);
  std::cout << "single data len: " << expData.reim.size() << ", num freqs: " << expData.freqs.size() << std::endl;

  for (int i = 0; i<avg.stdAmp.size(); i++) {
    
    if (i % 4 == 0) {std::cout << std::endl;}
    std::cout << avg.stdAmp[i] << ", ";
   }
    //for (int i = 0; i<avg.damp.size(); i++) {
    
    // if (i % 4 == 0) {std::cout << std::endl;}
    // std::cout << avg.damp[i] << ", ";
    // }
  // //  std::vector<std::complex<float>> aDat = getOneWavelengthComplex(expData,1); 

  //TESTING PARTIAL DERVIATIVES/////////////////////
  std::cout << "Testing Partial Derivatives" << std::endl;

  float freq = 50;
  float sep = 10;
  float dfdp[4];
  float dp = 0.0001;

  dfdp_p1seminf(785,freq,sep, dp, &dfdp[0]);

  std::cout << "dAmp/dMua: " << dfdp[0] << std::endl;
  std::cout << "dPhase/dMua: " << dfdp[1] << std::endl;
  std::cout << "dAmp/dMus: " << dfdp[2] << std::endl;
  std::cout << "dPhase/dMus: " << dfdp[3] << std::endl;
  ////////////////////////////////////////////////
  std::vector<float> acsdsqd;
  std::vector<float> phisdsqd;
  acsdsqd.reserve(avg.freqs.size());
  phisdsqd.reserve(avg.freqs.size());
  std::vector<std::complex<float>> sResp = sysResponseSweep(avg,&acsdsqd,&phisdsqd);
  for (int i = 0; i<4; i++) {
    std::cout << std::endl;
    for (int j = 0; j< 4; j++ ) {
      std::cout<<phisdsqd[i*4+j];
    }
  }
  std::vector<std::complex<float>> acal= calibrate(&expData.reim,&sResp);
  std::vector<std::complex<float>> trivialCal = calibrate(&avg.reim,&sResp);

  //expData.calReim=acal  // //std::vector<float> OPs =
  std::vector<float> reAvg, imAvg, reOne, imOne,reCal,imCal,reResp,imResp;
  std::vector<std::complex<float>> oneLamAvg= getOneWavelengthComplex(avg,3);
  std::vector<std::complex<float>> oneLamExp =getOneWavelengthComplex(expData,3);
  std::vector<std::complex<float>> oneLamCal=getOneWavelengthComplex(trivialCal,3,4,244);
  std::vector<std::complex<float>> oneLamResp=getOneWavelengthComplex(sResp,3,4,244);

  std::vector<float> recOP = runInverseModel(expData.SDSep, expData.freqs,oneLamCal,acsdsqd);
  std::cout << "mua: " << recOP[0] << ", mus: " << recOP[1] << std::endl;
  for (int q = 0; q<avg.freqs.size(); q++) {
    reAvg.push_back(oneLamAvg[q].real());
    imAvg.push_back(oneLamAvg[q].imag());
    reOne.push_back(oneLamExp[q].real());
    imOne.push_back(oneLamExp[q].imag());
    reCal.push_back(oneLamCal[q].real());
    imCal.push_back(oneLamCal[q].imag());
    reResp.push_back(oneLamResp[q].real());
    imResp.push_back(oneLamResp[q].imag());
    
  }
  //Test div vec
  //  std::vector<std::complex<float>> t1 = {{1,2},{3,4},{5,6}};
  // std::vector<std::complex<float>> t2 = {{1,0},{2,0},{3,0}};
  //std::vector<float> t3 = {1,3,5};
  //std::vector<float> t4 = {1,2,3};
  //std::vector<std::complex<float>> testDiv=divVecs(t1,t2);
  //std::vector<float> testDiff=diffVecs(t3,t4);
  //for (int i = 0; i<3; i++) {
  //  std::cout << testDiff[i] << std::endl;
  // }
  std::vector<float> avgReal,sRespReal,calReal;
  std::vector<float> x;
  for (int z = 0; z<avg.reim.size(); z++) {
    avgReal.push_back(avg.reim[z].real());
    sRespReal.push_back(sResp[z].real());
    calReal.push_back(avg.reim[z].real()/sResp[z].real());
    x.push_back(z);
  }
  //Plot the imaginary part
  Gnuplot g1("lines");
  g1.set_style("lines").plot_xy(avg.freqs,reCal);
  //g1.set_style("lines").plot_xy(avg.freqs,reResp);
  //g1.set_style("lines").plot_xy(x,avgReal);
  //g1.set_style("lines").plot_xy(x,sRespReal);
  g1.showonscreen();
  //wait_for_key();

  // std::cout<< "Num Freqs: "<<avg.freqs.size() <<", reRespLen: " << reResp.size() << std::endl;
  // //Plot the imaginary part
  // Gnuplot g2("lines");
  // //g2.set_style("points").plot_xy(avg.freqs,reAvg);
  // g2.set_style("lines").plot_xy(avg.freqs,reCal);
  // g2.showonscreen();
   wait_for_key();
 
  return 0;
}


void wait_for_key ()
{
  std::cout << std::endl << "Press a key" << std::endl;
  std::cin.clear();
  std::cin.ignore(std::cin.rdbuf()->in_avail());
  std::cin.get();
  return;
}
