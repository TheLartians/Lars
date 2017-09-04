#pragma once

namespace lars{
  
  using TypeIndex = unsigned;

  template <int CONTEXT = 0> struct TypeIndexContext{
    static lars::TypeIndex type_count;
    template <class T> struct Index{ static lars::TypeIndex value; };
  };
  
  template <int CONTEXT> TypeIndex TypeIndexContext<CONTEXT>::type_count = 0;
  template <int CONTEXT> template <class T> TypeIndex TypeIndexContext<CONTEXT>::Index<T>::value = TypeIndexContext<CONTEXT>::type_count++;
  
  template <class T> TypeIndex get_type_index(){
    return TypeIndexContext<>::Index<T>::value;
  }

}

