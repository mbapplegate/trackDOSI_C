
#include "FDPM.h"
#include <iostream>
#include "interpolation.h"
#include "gnuplot_i.h"
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

  
  int wavelength = 690;
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
  
  std::complex<float> complexRes=calcReImSystemResponse(wavelength, f, SDSep, fakeExpReIm);
  
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

  std::vector<float> sysRespAmp;
  std::vector<float> sysRespPhase;

  sysResponseSweep(wavelength, sweepFreq, SDSep, ampVals, phaseVals, &sysRespAmp, &sysRespPhase);

  std::vector<std::complex<float>> x= sysResponseSweep(wavelength, sweepFreq, SDSep, sweepComplex);
  //  for (size_t i =0; i<numFreqs; i++) {
  //  std::cout << x[i] <<", (" << sysRespAmp[i] << ", " << sysRespPhase[i] << ")"  <<std::endl;
  // }

  
  //Plot the imaginary part
  // Gnuplot g1("lines");
  // //g1.set_style("points").plot_xy(sweepFreq,realVals);
  // g1.set_style("points").plot_xy(sweepFreq,imagVals);
  // g1.showonscreen();
  // wait_for_key();
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
