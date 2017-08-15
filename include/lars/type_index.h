#pragma once

namespace lars{

  template <class Index = unsigned> struct TypeIndexContext{
    static Index type_count;
    template <class T> struct TypeIndex{ static Index value; };
  };
  
  template <class Index> Index TypeIndexContext<Index>::type_count = 0;
  template <class Index> template <class T> Index TypeIndexContext<Index>::TypeIndex<T>::value = TypeIndexContext<Index>::type_count++;
  
  template <class T> unsigned get_type_index(){
    return TypeIndexContext<>::TypeIndex<T>::value;
  }

}

