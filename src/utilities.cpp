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

//Element wise division of vectors
std::vector<float> divVecs(const std::vector<float>& a, const std::vector<float>& b) {
  assert(a.size() == b.size());

  std::vector<float> result;
  result.reserve(a.size());

  std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result),std::divides<float>());

  return result;
}

//Divide vector by scalar (probably not efficient)
std::vector<float> divVecs(const std::vector<float>& a, float b) {
  
  std::vector<float> result;
  result.reserve(a.size());

  for (size_t i = 0; i<a.size(); i++) {
    result[i] = a[i]/b;
  }

  return result;
}
