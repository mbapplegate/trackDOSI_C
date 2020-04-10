//Header file with utility functions
#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <complex>
std::vector<float> sumVecs(const std::vector<float>&, const std::vector<float>&);
std::vector<std::complex<float>> sumVecs(const std::vector<std::complex<float>>&, const std::vector<std::complex<float>>&);
std::vector<float> diffVecs(const std::vector<float>&, const std::vector<float>&);
std::vector<float> divVecs(const std::vector<float>&, const std::vector<float>&);
std::vector<std::complex<float>> divVecs(const std::vector<std::complex<float>>&, const std::vector<std::complex<float>>&);
std::vector<float> divVecs(const std::vector<float>&, float);
std::vector<std::complex<float>> divVecs(const std::vector<std::complex<float>>&, float);
#endif
