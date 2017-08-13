#pragma once

#include <limits>

namespace lars {
  
  template <typename T> inline T abs(const T& v) {
    return v < 0 ? -v : v;
  }
  
  template<class T> inline constexpr T pow(const T base, const unsigned exponent) {
    return (exponent == 0) ? 1 : (exponent % 2 == 0) ? pow(base, exponent/2)*pow(base, exponent/2) : base * pow(base, (exponent-1)/2) * pow(base, (exponent-1)/2);
  }
  
  template<unsigned exponent,class T> inline constexpr T pow(const T base) {
    return (exponent == 0) ? 1 : (exponent % 2 == 0) ? pow<exponent/2>(base)*pow<exponent/2>(base) : base * pow<(exponent-1)/2>(base) * pow<(exponent-1)/2>(base);
  }

  template <typename T> static int sign(T val) {
    return (T(0) < val) - (val < T(0));
  }
  
  template <typename T> bool are_approximately_equal(T a, T b, T epsilon = std::numeric_limits<T>::epsilon()){
    return lars::abs(a - b) <= ( (lars::abs(a) < lars::abs(b) ? lars::abs(b) : lars::abs(a)) * epsilon);
  }
  
  template <typename T> bool are_essentially_equal(T a, T b, T epsilon = std::numeric_limits<T>::epsilon()){
    return fabs(a - b) <= ( (lars::abs(a) > lars::abs(b) ? lars::abs(b) : lars::abs(a)) * epsilon);
  }
  
  template <typename T> bool definitely_greater_than(T a, T b, T epsilon = std::numeric_limits<T>::epsilon()){
    return (a - b) > ( (lars::abs(a) < lars::abs(b) ? lars::abs(b) : lars::abs(a)) * epsilon);
  }
  
  template <typename T> bool definitely_less_than(T a, T b, T epsilon = std::numeric_limits<T>::epsilon()){
    return (b - a) > ( (lars::abs(a) < lars::abs(b) ? lars::abs(b) : lars::abs(a)) * epsilon);
  }
  

}
