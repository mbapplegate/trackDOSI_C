/*FDPM.c
 *
 *PURPOSE: A place to house functions related to Frequency Domain Photon Migration. Including models, calibration routines, and (hopefully) inverse models
 *
 *Author: Matt Applegate
 *Date Created: 29 October 2019
 *Last Modified: 10 April 2020
 *
 *Updated to use std::vector instead of arrays
 */

#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <complex>
#include "FDPM.h"
#include "interpolation.h"
#include "utilities.h"
#include "ASCData.h"

//using namespace alglib;
/*complex double
 *p1SemiInf
 *
 *PURPOSE: Calculates the P1 semiinfinite diffusion approximation for a given OP and frequency. Returns real and imaginary parts
 *
 *INPUTS:
 *  mua   //Absorption coefficient (1/mm)
 *  mus   //Reduced scattering coefficient (1/mm)
 *  f     //Modulation frequency (MHz)
 *  SDSep //Source-detector separation (MHz)
 *
 *OUTPUTS:
 *  val   //Complex diffusion approximation (not sure of units tbh)
 *
 *NOTES: This was copied from p1seminf.m which itself was based on:
 *Cerrusi and Tromberg (2003), Photon Migration Spectroscopy: Frequency Domain Techniques. Biomedical Photonics Handbook. T. Vo-Dinh,
 *CRC Press: 22.1-22.17
 *
 *It may have been modified by AEC and Josh, but I don't know who they are
 */
std::complex<float> p1SemiInf(float mua, float mus, float f, float SDSep) {

  float ior = 1.4; //Index of refraction, fixed for now
  float c = 2.99792457e11/ior; //Speed of light in mm/s
  float mutr = mua+mus; //mu transport in 1/mm
  float ltr = 1/mutr; // 1/mu transport in mm
  float D=(1/3.0) * ltr; //Diffusion Constant in mm
  //fprintf(stderr, "c=%f, mutr=%f, ltr =%f, D=%f\n",c,mutr,ltr,D);
  float fbc = 1.0e6*2*M_PI*f/c; //angular frequency of f/c (rad/mm)
  float alpha = 3*fbc*D;      //Not sure what this is honestly (radians)

  float reff = 0.493; //Copie from p1seminf.m

  float zb = 2/3.0 * (1+reff)/(1-reff) * ltr; //Zero flux boundary location (mm)
  //fprintf(stderr,"zb=%f, alpha=%f, fbc=%f\n",zb,alpha,fbc);
  //Distance to source and image (mm)
  float r01 = sqrt(ltr*ltr+SDSep*SDSep);
  float rb1 = sqrt((2*zb+ltr)*(2*zb+ltr)+SDSep*SDSep);

  //fprintf(stderr, "R01=%f, RB1= %f\n",r01,rb1);
  std::complex<float> k((mua-fbc*alpha)/D,-(fbc+mua*alpha)/D);
  k = sqrt(k);
  float kr = std::abs(k.real());
  float ki = std::abs(k.imag());
  //fprintf(stderr, "kr=%f, ki=%f\n",kr,ki);
  float er01 = exp(-kr*r01);
  float erb1 = exp(-kr*rb1);
  //fprintf(stderr, "er01=%f, erb1=%f\n",er01,erb1);
  std::complex<float> val(((er01/r01) * cos(ki*r01) - (erb1/rb1)*cos(ki*rb1))/D,
			   ((er01/r01) * sin(ki*r01) - (erb1/rb1)*sin(ki*rb1))/D);
  return val;
 
}


std::complex<float> p1SemiInf(const alglib::real_1d_array &op, float f, float SDSep) {
  float ior = 1.4; //Index of refraction, fixed for now
  float c = 2.99792457e11/ior; //Speed of light in mm/s
  float mutr = op[0]+op[1]; //mu transport in 1/mm
  float ltr = 1/mutr; // 1/mu transport in mm
  float D=(1/3.0) * ltr; //Diffusion Constant in mm
  //fprintf(stderr, "c=%f, mutr=%f, ltr =%f, D=%f\n",c,mutr,ltr,D);
  float fbc = 1.0e6*2*M_PI*f/c; //angular frequency of f/c (rad/mm)
  float alpha = 3*fbc*D;      //Not sure what this is honestly (radians)

  float reff = 0.493; //Copie from p1seminf.m

  float zb = 2/3.0 * (1+reff)/(1-reff) * ltr; //Zero flux boundary location (mm)
  //fprintf(stderr,"zb=%f, alpha=%f, fbc=%f\n",zb,alpha,fbc);
  //Distance to source and image (mm)
  float r01 = sqrt(ltr*ltr+SDSep*SDSep);
  float rb1 = sqrt((2*zb+ltr)*(2*zb+ltr)+SDSep*SDSep);

  //fprintf(stderr, "R01=%f, RB1= %f\n",r01,rb1);
  std::complex<float> k((op[0]-fbc*alpha)/D,-(fbc+op[0]*alpha)/D);
  k = sqrt(k);
  float kr = std::abs(k.real());
  float ki = std::abs(k.imag());
  //fprintf(stderr, "kr=%f, ki=%f\n",kr,ki);
  float er01 = exp(-kr*r01);
  float erb1 = exp(-kr*rb1);
  //fprintf(stderr, "er01=%f, erb1=%f\n",er01,erb1);
  std::complex<float> val(((er01/r01) * cos(ki*r01) - (erb1/rb1)*cos(ki*rb1))/D,
			   ((er01/r01) * sin(ki*r01) - (erb1/rb1)*sin(ki*rb1))/D);

  return val;
 
 
}



/*void
 *getModelError
 *
 *PURPOSE: Calculates the difference between the measured and model data
 *
 */
void getModelError(const alglib::real_1d_array &op, alglib::real_1d_array &fi, void *ptr) {
  //fprintf(stderr,"%f, %f\n", op[0], op[1]);
  inverseData passedDat = *(inverseData *)(ptr); 
  
  //std::complex<float> p1Res;
  std::vector<std::complex<float>> p1Res = p1Sweep(op[0],op[1],passedDat.freqs, passedDat.SDSep);
  //float rePart;
  //float imPart;
  for (size_t i =0; i< passedDat.freqs.size(); i++) {
    //rePart = passedDat.calReim[i] * cos(passedDat.phase[i]);
    //imPart = passedDat.amp[i] * sin(passedDat.phase[i]);
    fi[2*i] = (p1Res[i].real() - passedDat.calDat[i].real())*passedDat.wts[2*i];
    fi[2*i+1] = (p1Res[i].imag() - passedDat.calDat[i].imag())*passedDat.wts[2*i+1];
    //std::complex<float> d = p1Res[i]-passedDat.calReim[i];
    //fi[i] = std::abs(d);
  }
  //printf("%s %f %f\n",op.tostring(2).c_str(),p1Res.real(), p1Res.imag());
}


/*float
 *chi
 *
 *PURPOSE: Calculates the cost function for the gradient descent algorithm
 *
 */
float chi(const alglib::real_1d_array &op, std::vector<std::complex<float>> measuredDat,float SDsep, std::vector<float> f) {
  //std::complex<float> *simDat = new std::complex<float>[numFreqs];
  std::vector<std::complex<float>> simDat = p1Sweep(op[0],op[1],f,SDsep);
  float ss = 0;
  for (size_t i = 0; i<f.size(); i++) {
    std::complex<float> x= simDat[i] - (std::complex<float>)measuredDat[i];
    ss+= std::abs(x);
  }
  //delete[] simDat;
  return ss/(f.size()-2);
}

std::vector<float> wtsAmpPhase2ReIm(std::vector<float> ACsd_sqd, std::vector<float> Phisd_sqd,const ASCData *rawDat, std::vector<std::complex<float>> calResp) {

  std::vector<float> calAmp, calPhase;
  calAmp.reserve(ACsd_sqd.size());
  calPhase.reserve(ACsd_sqd.size());

  for (size_t i = 0; i<ACsd_sqd.size(); i++) {
    float tempAmp, tempPhase;
    ReIm2AmpPhase(calResp[i], &tempAmp, &tempPhase);
    calAmp.push_back(tempAmp);
    calPhase.push_back(tempPhase);
    //ReIm2AmpPhase(rawDat[i], &tempAmp2, &tempPhase2);
  }

  //std::cout << "size of stdAmp: " << rawDat->stdAmp.size() << std::endl;
  std::vector<float> damp = multVecs(calAmp,sqrtVecs(sumVecs(squareVecs(divVecs(rawDat->stdAmp,rawDat->amp)),ACsd_sqd)));
  std::vector<float> dphi = sqrtVecs(sumVecs(Phisd_sqd,squareVecs(rawDat->stdPhase)));
  
  std::vector<float> dReal_term1 = squareVecs(multVecs(damp,cosVecs(calPhase)));
  std::vector<float> dReal_term2 = squareVecs(multVecs(dphi,multVecs(calAmp,sinVecs(calPhase))));
  std::vector<float> dImag_term1 = squareVecs(multVecs(damp, sinVecs(calPhase)));
  std::vector<float> dImag_term2 = squareVecs(multVecs(dphi,multVecs(calAmp,cosVecs(calPhase))));

  std::vector<float> dReal = sqrtVecs(sumVecs(dReal_term1,dReal_term2));
  std::vector<float> dImag = sqrtVecs(sumVecs(dImag_term1, dImag_term2));

  std::vector<float> result;
  result.reserve(2*ACsd_sqd.size());
  for (size_t i = 0; i<ACsd_sqd.size(); i++) {
    result.push_back(1/dReal[i]);
    result.push_back(1/dImag[i]);
  }
  return result;
}
std::vector<float> runInverseModel(float SDSep, std::vector<float> freqs, std::vector<std::complex<float>> dat, std::vector<float> wts, int numLams){

  
  const int numStarts = 2;
  float chis[numStarts];
  //real_1d_array x;
  alglib::real_1d_array q0;
  alglib::real_1d_array q1;
  float muaGuess[numStarts] = { 0.005, .001 };// {.005, .001, .01, .05, .005};
  float musGuess[numStarts] = { .8,1.3 };// {.8, 1.3, 1.0, 1.0, .6};
  float muaRec[numStarts];
  float musRec[numStarts];

  inverseData d;
  d.SDSep = SDSep;
  d.freqs = freqs;
  int numFreqs = (int)freqs.size();

  
  std::vector<float> OP;
  OP.reserve(2*numLams);

  for (int i = 0; i<numLams; i++) {
    std::vector<std::complex<float>> oneLamCal=getOneWavelengthComplex(dat,i,numLams,numFreqs);
    std::vector<float> oneLamWts = getOneWavelengthWts(wts, i, numLams, numFreqs);
    d.calDat = oneLamCal;
    d.wts = wts;  
    //printf("True values: [%f,%f]\n",randMua,randMus);
    for (int q= 0; q<numStarts; q++) {
     
      char buf[20];
      sprintf(buf,"[%.4f,%.4f]",muaGuess[q],musGuess[q]);
      alglib::real_1d_array x=buf;
    
      //fprintf(stderr,"q=%d\n",q); 
      alglib::minlmstate state;
      alglib::minlmreport rep;
      float epsx = 1e-12;
      alglib::ae_int_t maxits = 0;
      alglib::real_1d_array bndl = "[.00001, .001]";
      alglib::real_1d_array bndu = "[.5, 5]";
      alglib::real_1d_array s = "[.01,1]";
      //fprintf(stderr,"%f\n",x[0]);
      alglib::minlmcreatev(2,2*numFreqs,x,0.0001,state);
      alglib::minlmsetbc(state,bndl,bndu);
      alglib::minlmsetcond(state,epsx,maxits);
      alglib::minlmsetscale(state,s);
      
      alglib::minlmoptimize(state,getModelError,NULL,&d);
      alglib::minlmresults(state,x,rep);
      chis[q] = chi(x,dat,d.SDSep,d.freqs);
      muaRec[q] = x[0];
      musRec[q] = x[1];
    }
    int bestIdx = 0;
    float bestCHI=10000;
    for (uint32_t i = 0; i< numStarts; i++) {
      if (chis[i] < bestCHI) {
	bestIdx = i;
	bestCHI = chis[i];
      }
    }

    OP.push_back(muaRec[bestIdx]);
    OP.push_back(musRec[bestIdx]);
  }
  return OP;
}

/*void
 *p1Sweep
 *
 *PURPOSE: returns vector of P1 approximation given a vector of modulation frequencies
 *
 *INPUTS:
 * mua       //Absorption coefficient (1/mm)
 * mus       //Reduced scattering coefficient (1/mm)
 * f         //VECTOR of modulation frequencies (MHz)
 * SDsep     //SD separation (mm)
 * numFreqs  //length of f
 *OUTPUTS:
 * result    //COMPLEX vector of p1 diffusion approximation
 *
 */
//std::vector<std::complex<float>> p1Sweep(float mua, float mus, std::vector<float> f, float SDsep){
//  std::vector<std::complex<float>> result;
// result.reserve(f.size());
// for (size_t i = 0; i<f.size(); i++){
//   result.push_back( (std::complex<float>)p1SemiInf(mua,mus,f[i],SDsep));
// }
// return result;
//}

std::vector<std::complex<float>> p1Sweep(float mua, float mus, std::vector<float> f, float SDsep){
  std::vector<std::complex<float>> result;
  result.reserve(f.size());
  for (size_t i = 0; i<f.size(); i++){
    result.push_back( p1SemiInf(mua,mus,f[i],SDsep));
  }
  return result;
}

/*void
 *ReIm2AmpPhase
 *
 *PURPOSE: Converts Complex values to amplitude and phase
 *
 *Inputs:
 * z    //Complex double from P1 approximation
 *
 *OUTPUTS:
 * amp   //Amplitude
 * phase //Phase angle atan2(imag/real)
 *
 */
void ReIm2AmpPhase(std::complex<float> z, float* amp, float* phase) {
  *amp = std::abs(z);
  *phase = std::arg(z);
}

void ReIm2AmpPhase(std::complex<double> z, double* amp, double* phase) {
  *amp = std::abs(z);
  *phase = std::arg(z);
}

/*complex double
 *AmpPhase2ReIm
 *
 *PURPOSE: Converts amplitude and phase to real and imaginary. Mostly included for completeness as there is a built in function that does the same thing
 *
 */
std::complex<float> AmpPhase2ReIm(float amp, float phase) {
  //std::complex<double> z(amp * cos(phase), amp*sin(phase));
  return std::polar(amp,phase);
}

/*void
 *getACRINops
 *
 *PURPOSE: Return tabulated absorption and scattering coefficients from the ACRIN9.txt file
 *This is meant to capture the most common situation in the lab at this time. 
 *It is not meant to be a universal OP getter with filename parsing and such
 *
 *INPUTS:
 * wavelength  //The wavelength you want the OPs from (only takes integral numbers of nm)
 *OUTPUTS:
 * mua         //Absorption coefficient of ACRIN at that wavelength (1/mm)
 * mus         //Reduced scattering coefficient of ACRIN at that wavelength (1/mm)
 *
 */
void getACRINops(int wavelength, float* mua, float* mus) {
  switch(wavelength) {
  case 658:
    *mua = 0.018416369;
    *mus = 0.803474193;
    break;
  case 690:
    *mua = 0.015987247;
    *mus = 0.769387442;
    break;
  case 785:
    *mua = 0.009074767;
    *mus = 0.683981478;
    break;
  case 808:
    *mua = 0.008116152;
    *mus = 0.666211485;
    break;
  case 830:
    *mua = 0.006885851;
    *mus = 0.650099902;
    break;
  case 850:
    *mua = 0.006557937;
    *mus = 0.636148515;
    break;
  default:
    //TODO: Add something that reads the ACRIN9.txt file and gives a sensible value here
    //0s should yield nans which will tell you something went wrong
    *mua = 0;
    *mus = 0;
    fprintf(stderr, "Other wavelengths not implemented yet\n");
  }
}



/*void
 *calcAmpPhaseSystemResponse
 *
 *PURPOSE: Calculates the system response assuming ACRIN9 is used as a phantom and returns the response in terms of the real and imaginary parts
 *
 *INPUTS:
 * wavelength  //Wavelength of interest (nm)
 * f           //frequency used (MHz)
 * SDSep       //Source-detector separation (mm)
 * expAmp      //Experimentally measured amplitude at this frequency/SDsep
 * expPhase    //Experimentally measured phase (rad)
 *
 *OUTPUTS:
 * sysRespAmp   //System response for this wavelength, frequency and SD Sep
 * sysRespPhase
 *
 */
void calcAmpPhaseSystemResponse(int wavelength, float f, float SDSep,float expAmp, float expPhase, float* sysRespAmp, float* sysRespPhase) {
  float phantomMua;
  float phantomMus;
  float phantomAmp;
  float phantomPhase;
  getACRINops(wavelength, &phantomMua, &phantomMus);
  std::complex<float> p1Result = p1SemiInf(phantomMua, phantomMus,f,SDSep);

  ReIm2AmpPhase(p1Result,&phantomAmp, &phantomPhase);

  *sysRespAmp = expAmp / phantomAmp;
  *sysRespPhase = expPhase - phantomPhase;
}


/*complex double
 *calcReImSystemResponse
 *
 *PURPOSE: Same as above but uses the real and imaginary parts instead of the amplitude and phase
 *
 *NOTES: This hasn't been thoroughly tested so I'm not sure if it's accurate or not. The subtraction of phase really throws me off. I think it should work though
 *
 *INPUTS:
 *  wavelength //Wavelength used (nm)
 *  f          //Frequency used (MHz)
 *  SDSep      //Source-detector separation (mm)
 *  expResult  //Complex experimental result
 *
 *OUTPUTS
 * systemResponse //Complex system response
 *
 */

std::complex<float> calcReImSystemResponse(int wavelength, float f, float SDSep, std::complex<float> expResult, std::complex<float>* theory) {
  float phantomMua;
  float phantomMus;
  getACRINops(wavelength, &phantomMua, &phantomMus);
  std::complex<float> phantomResult = p1SemiInf(phantomMua, phantomMus, f, SDSep);
  //std::cout << "phantomResult: " << wavelength << std::endl;
  //	    <<"expResult: " << expResult << std::endl;
  *theory = phantomResult;
  return (std::complex<float>)expResult/phantomResult;
}


void sysResponseSweep(int wavelength, std::vector<float> f, float SDSep, std::vector<float> expAmpVec, std::vector<float> expPhaseVec, std::vector<float>* sysRespAmp, std::vector<float>* sysRespPhase) {
  sysRespAmp->reserve(f.size());
  sysRespPhase->reserve(f.size());
  float thisAmp;
  float thisPhase;
  for (size_t i = 0; i<f.size(); i++) {
    calcAmpPhaseSystemResponse(wavelength, f[i], SDSep, expAmpVec[i],expPhaseVec[i], &thisAmp, &thisPhase);
    sysRespAmp->push_back(thisAmp);
    sysRespPhase->push_back(thisPhase);
  }
}

std::vector<std::complex<float>> sysResponseSweep(ASCData expResult,std::vector<float>* ACsdsqd, std::vector<float>* Phisdsqd) {
  //float pdmua = 0.05;
  //float pdmus = 0.05;
  
  int numWavelengths = (int)expResult.wavelengths.size();
  int numFreqs = (int)expResult.freqs.size();
  std::vector<std::complex<float>> sysResponse;
  std::vector<float> dAmpdMua;
  std::vector<float> dPhasedMua;
  std::vector<float> dAmpdMus;
  std::vector<float> dPhasedMus;
  std::vector<float> theoryAmp;
  std::vector<float> theoryPhase;
  std::vector<float> pdmua;
  std::vector<float> pdmus;

  dAmpdMua.reserve(numFreqs*numWavelengths);
  dAmpdMus.reserve(numFreqs*numWavelengths);
  dPhasedMua.reserve(numFreqs*numWavelengths);
  dPhasedMus.reserve(numFreqs*numWavelengths);
  sysResponse.reserve(numFreqs*numWavelengths);
  theoryAmp.reserve(numFreqs*numWavelengths);
  theoryPhase.reserve(numFreqs*numWavelengths);
  
  float oneDFDP[4];
  
  for (int i = 0; i< numFreqs; i++) {
    for (int j = 0; j<numWavelengths; j++) {      
      std::complex<float> theory;
      std::complex<float> thisResp = calcReImSystemResponse(expResult.wavelengths[j], expResult.freqs[i], expResult.SDSep, expResult.reim[i*numWavelengths+j], &theory);
      sysResponse.push_back(thisResp);
      dfdp_p1seminf(expResult.wavelengths[j],expResult.freqs[i],expResult.SDSep,0.0001,&oneDFDP[0]);
      dAmpdMua.push_back(oneDFDP[0]);
      dPhasedMua.push_back(oneDFDP[1]);
      dAmpdMus.push_back(oneDFDP[2]);
      dPhasedMus.push_back(oneDFDP[3]);

      float thisAmp,thisPhase;
      ReIm2AmpPhase(theory, &thisAmp, &thisPhase);
      theoryAmp.push_back(thisAmp);
      theoryPhase.push_back(thisPhase);

      float thismua, thismus;
      getACRINops(expResult.wavelengths[j],&thismua,&thismus);
      pdmua.push_back(thismua * 0.05);
      pdmus.push_back(thismus *0.05);
      //std::cout << oneDFDP[0] << std::endl;
    }
  }

  std::vector<float> acsdsqd_firstterm = squareVecs(divVecs(expResult.stdAmp,expResult.amp));
  
  std::vector<float> acsdsqd_secondterm =divVecs(sumVecs(squareVecs(multVecs(pdmua,dAmpdMua)),squareVecs(multVecs(pdmus,dAmpdMus))),squareVecs(theoryAmp));
  //std::vector<float> acsdsqd_st = multVecs(pdmus,dAmpdMus);
  std::vector<float> phisdsqd_firstterm = squareVecs(expResult.stdPhase);
  std::vector<float> phisdsqd_secondterm = squareVecs(multVecs(dPhasedMua,pdmua));
  //  std::vector<float> acsdsqd_thirdterm = squareVecs(divVecs(multVecs(dAmpdMus, pdmus),theoryAmp));
  std::vector<float> phisdsqd_thirdterm = squareVecs(multVecs(dPhasedMus,pdmus));

  *ACsdsqd = sumVecs(acsdsqd_firstterm,acsdsqd_secondterm);
  //*Phisdsqd = sumVecs(phisdsqd_firstterm,phisdsqd_secondterm);
  //*ACsdsqd = sumVecs(sumVecs(acsdsqd_firstterm,acsdsqd_secondterm),acsdsqd_thirdterm);
  *Phisdsqd = sumVecs(sumVecs(phisdsqd_firstterm,phisdsqd_secondterm), phisdsqd_thirdterm);
  return sysResponse;
}


void dfdp_p1seminf(float wavelength, float freq, float sep,float dp, float* dfdp) {
  //int m = 2;
  //int n = 2;
  
  float mua_start;
  float mus_start;
  getACRINops(wavelength,&mua_start,&mus_start);
  float mua = mua_start;
  float mus = mus_start;
  std::complex<float> f = p1SemiInf(mua,mus,freq,sep);
  float fAmp;
  float fPhase;
  ReIm2AmpPhase(f,&fAmp,&fPhase);
  float nabMua = mua * dp;
  if ( mua == 0) {
    nabMua = dp;
  }
  mua = mua_start + nabMua;
  if (nabMua != 0) {
    std::complex<float> f1 = p1SemiInf(mua,mus,freq,sep);
    float f1Amp;
    float f1Phase;
    ReIm2AmpPhase(f1, &f1Amp,&f1Phase);
        if (dp < 0) {
      dfdp[0] = (f1Amp - fAmp)/nabMua;
      dfdp[1] = (f1Phase - fPhase)/nabMua;
    }
    else {
      mua = mua_start - nabMua;
      //std::cout << "nabla mua: " << nabMua << std::endl;
      std::complex<float> f2 = p1SemiInf(mua,mus,freq,sep);
      float f2Amp;
      float f2Phase;
      ReIm2AmpPhase(f2,&f2Amp,&f2Phase);
      dfdp[0] = (f1Amp-f2Amp)/(2*nabMua);
      dfdp[1] = (f1Phase-f2Phase)/(2*nabMua);
      
      //std::cout << "F1 Amp: " << f1Amp << std::endl <<
      //"F1 Phase: " << f1Phase << std::endl;
      //std::cout << "F2 Amp: " << f2Amp << std::endl <<
      //"F2 Phase: " << f2Phase << std::endl;

    }
  }
  else {
    dfdp[0] = 0;
    dfdp[1] = 0;
  }
  
  mua = mua_start;

  float nabMus = dp * mus;
  if(mus == 0) {
    nabMus = dp;
  }
  mus = mus_start + nabMus;
  if (nabMus != 0) {
    std::complex<float> f1 = p1SemiInf(mua,mus,freq,sep);
    float f1Amp, f1Phase;
    ReIm2AmpPhase(f1,&f1Amp, &f1Phase);
    if (dp < 0) {
      dfdp[2] = (f1Amp - fAmp)/nabMus;
      dfdp[3] = (f1Phase - fPhase)/nabMus;
    }
    else {
      mus = mus_start - nabMus;
      std::complex<float> f2 = p1SemiInf(mua,mus,freq,sep);
      float f2Amp, f2Phase;
      ReIm2AmpPhase(f2,&f2Amp,&f2Phase);
      dfdp[2] = (f1Amp - f2Amp)/(2*nabMus);
      dfdp[3] = (f1Phase - f2Phase)/ (2*nabMus);
    }
    
  }
  else{
    dfdp[2] = 0;
    dfdp[3] = 0;
  }
}

void calibrate(std::vector<float> Amp, std::vector<float> Phase, std::vector<float> sysRespAmp, std::vector<float> sysRespPhase, std::vector<float>* calAmp, std::vector<float>* calPhase){

  *calAmp = divVecs(Amp,sysRespAmp);
  *calPhase = diffVecs(Phase,sysRespPhase);

}

std::vector<std::complex<float>> calibrate(const std::vector<std::complex<float>>* expResult, const std::vector<std::complex<float>>* sysResponse) {

  std::vector<std::complex<float>> calResp = divVecs(*expResult,*sysResponse);
  return calResp;
  
}

//std::vector<float> dfdp(std::vector<float> freqs, std::vector<float> theoAmp, float mua, float mus, float prec, float n, float SDSep) {
//  int m = 2*freqs.size();
//  int n = 2;


// //void p1SemiInfLM(const real_1d_array &c constant_real_1d_array &x,double &func, void *ptr) {

// //  float SDsep = 10;
// // float ior = 1.4; //Index of refraction, fixed for now
// // double c = 2.99792457e11/ior; //Speed of light in mm/s
// // double mutr = c[0]+c[1]; //mu transport in 1/mm
// // double ltr = 1/mutr; // 1/mu transport in mm
// // double D=(1/3.0) * ltr; //Diffusion Constant in mm
//   //fprintf(stderr, "c=%f, mutr=%f, ltr =%f, D=%f\n",c,mutr,ltr,D);
// //  double fbc = 1.0e6*2*M_PI*f[0]/c; //angular frequency of f/c (rad/mm)
// // double alpha = 3*fbc*D;      //Not sure what this is honestly (radians)

// // double reff = 0.493; //Copie from p1seminf.m
// //
// ///  double zb = 2/3.0 * (1+reff)/(1-reff) * ltr; //Zero flux boundary location (mm)
//   //fprintf(stderr,"zb=%f, alpha=%f, fbc=%f\n",zb,alpha,fbc);
//   //Distance to source and image (mm)
// // double r01 = sqrt(ltr*ltr+SDSep*SDSep);
// //  double rb1 = sqrt((2*zb+ltr)*(2*zb+ltr)+SDSep*SDSep);

//   //fprintf(stderr, "R01=%f, RB1= %f\n",r01,rb1);
// //  std::complex<double> k((c[0]-fbc*alpha)/D,-(fbc+c[0]*alpha)/D);
// //  k = sqrt(k);
// //  double kr = std::abs(k.real());
// //  double ki = std::abs(k.imag());
//   //fprintf(stderr, "kr=%f, ki=%f\n",kr,ki);
// //  double er01 = exp(-kr*r01);
// //  double erb1 = exp(-kr*rb1);
// //  //fprintf(stderr, "er01=%f, erb1=%f\n",er01,erb1);
// //  std::complex<double> val(((er01/r01) * cos(ki*r01) - (erb1/rb1)*cos(ki*rb1))/D,
// //			   ((er01/r01) * sin(ki*r01) - (erb1/rb1)*sin(ki*rb1))/D);
// //  func[0] = val.real();
// //  func[1] = val.imag();
// //}
// ///
// //int main(void){
// //  real_1d_array x = "[50,60,70,80,90,100,110,120,130,140]";
// //  real_2d_array y = "[[.0004705,.0000194],[.0004703,.0000233],[.0004700,.0000271],[.0004698,.0000310],[.0004691,.0000387],[.0004691,.0000387],[.0004688,0.0426],[.0004684,.0000464],[.0004679,.0000503],[.0004674,.0000541]]";
// //  real_1d_array c = "[.2,2]";
// //  double epsx 0.000001;
// //  ae_int_t maxits = 0;
// //  ae_int_t info;
// //  lsfitstate state;
// //  lsfitreport rep;
// //  double diffstep = 0.0001;

// //  lsfitcreatef(x,y,c,diffstep,state);
// //  lsfitsetcond(state, epsx, maxits);
// //  alglib::lsfitfit(state,P1SemiInfLM);
// //  lsfitresults(state, info, c, rep);
// //  printf("%d\n", int(info));
// //  printf("%s, %s\n", c[0].tostring(1).c_str(), c[1].tostring(1).c_str());
// //
// //  return 1;
// //}
 

//
