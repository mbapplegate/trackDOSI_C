//Utility functions that are broadly useful
#include <algorithm>
#include<functional>
#include "utilities.h"
#include <cassert>

//Element wise summation of two vectors

std::vector<float> sumVecs(const std::vector<float>& a, const std::vector<float>& b) {
  assert(a.size() == b.size());

  std::vector<float> result;
  result.reserve(a.size());

  std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result),std::plus<float>());

  return result;
}

//Elementwise summation of complex vector
std::vector<std::complex<float>> sumVecs(const std::vector<std::complex<float>>& a, const std::vector<std::complex<float>>& b) {
  assert(a.size() == b.size());

  std::vector<std::complex<float>> result;
  result.reserve(a.size());

  std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result),std::plus<std::complex<float>>());

  return result;
}


//Element wise subtraction of two vectors

std::vector<float> diffVecs(const std::vector<float>& a, const std::vector<float>& b) {
  assert(a.size() == b.size());

  std::vector<float> result;
  result.reserve(a.size());

  std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result),std::minus<float>());

  return result;
}

//Element wise division of vectors
std::vector<float> divVecs(const std::vector<float>& a, const std::vector<float>& b) {
  assert(a.size() == b.size());

  std::vector<float> result;
  result.reserve(a.size());

  std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result),std::divides<float>());

  return result;
}

//Element wise division of vectors
std::vector<std::complex<float>> divVecs(const std::vector<std::complex<float>>& a, const std::vector<std::complex<float>>& b) {
  assert(a.size() == b.size());

  std::vector<std::complex<float>> result;
  result.reserve(a.size());

  std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result),std::divides<std::complex<float>>());

  return result;
}

//Divide vector by scalar (probably not efficient)
std::vector<float> divVecs(const std::vector<float>& a, float b) {
  
  std::vector<float> result;
  result.reserve(a.size());

  for (size_t i = 0; i<a.size(); i++) {
    result.push_back( a[i]/b);
  }

  return result;
}


//Divide Complex vector by scalar (probably not efficient)
std::vector<std::complex<float>> divVecs(const std::vector<std::complex<float>>& a, std::complex<float> b) {
  
  std::vector<std::complex<float>> result;
  result.reserve(a.size());

  for (size_t i = 0; i<a.size(); i++) {
    result.push_back( a[i]/b);
  }

  return result;
}
