/*FDPM.h
 *
 *Header file for FDPM.c
 *
 *Author: Matt Applegate
 *Date Created: 29 October 2019
 *Last Modified: 9 April 2020
 */

#ifndef FDPM_H
#define FDPM_H

#include<complex>
#include<cmath>
#include<stdint.h>
#include<vector>
#include "interpolation.h"

//struct data {
//  float SDSep;
//  std::vector<float> freq;
//  int numFreqs;
//  std::vector<float> realPart;
//  std::vector<float> imPart;
//};

std::complex<float> p1SemiInf(float,float,float,float);
/* std::complex<float> p1SemiInf(float*, float,float); */
std::complex<float> p1SemiInf(const alglib::real_1d_array&, float, float);

/* void getModelError(const alglib::real_1d_array&, alglib::real_1d_array&, void*); */
/* float chi(const alglib::real_1d_array&, std::complex<float>*, int, float, float*); */

/* std::vector<float> runInverseModel(); */

std::vector<std::complex<float>> p1Sweep(float, float,std::vector<float>, float);

void ReIm2AmpPhase(std::complex<float>, float*, float*); 
std::complex<float> AmpPhase2ReIm(float, float);

void getACRINops(int, float*, float*);

void calcAmpPhaseSystemResponse(int, float, float, float, float, float*, float*);
std::complex<float> calcReImSystemResponse(int, float, float, std::complex<float>);

void sysResponseSweep(int, std::vector<float>, float,std::vector<float>, std::vector<float>, std::vector<float>*, std::vector<float>*);

std::vector<std::complex<float>> sysResponseSweep(int, std::vector<float>, float, std::vector<std::complex<float>>); 

/* void calibrate(float*, float*, float*, float*, uint32_t, float*, float*); */
/* void calibrate(std::complex<float>*, std::complex<float>*, uint32_t, std::complex<float>*); */
#endif