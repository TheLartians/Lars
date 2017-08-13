#pragma once

namespace lars{


  template <class Index = unsigned> struct TypeIndexContext{
    static Index type_count;
    template <class T> static Index type_index;
  };
  
  template <class Index> Index TypeIndexContext<Index>::type_count = 0;
  template <class Index> template <class T> Index TypeIndexContext<Index>::type_index = TypeIndexContext<Index>::type_count++;
  

}

