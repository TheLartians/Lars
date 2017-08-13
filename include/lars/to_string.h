
#pragma once

#include <sstream>

namespace lars {
  
  template <class T> std::string to_string(const T &obj,int precision = 0){
    std::stringstream stream;
    if(precision) stream.precision(precision);
    stream << obj;
    return stream.str();
  }

  template <typename T> std::wstring to_wstring(T value,int precision = 0 ) {
    std::wostringstream stream ;
    if(precision) stream.precision(precision);
    stream << value ;
    return stream.str() ;
  }
  
  template <typename T> T from_wstring(const std::wstring &string){
    T value = T();
    std::wistringstream(string) >> std::boolalpha >> value;
    return value;
  }
  
  template <typename T> T from_string(const std::string &string){
    T value = T();
    std::istringstream(string) >> std::boolalpha >> value;
    return value;
  }
  
  
}
