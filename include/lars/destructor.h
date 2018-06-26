#pragma once

#include <memory>

namespace lars {
  
  template <class F> struct Destructor{
    F callback;
    Destructor(const F & f):callback(f){}
    ~Destructor(){ callback(); }
  };
  
  template <class F> std::shared_ptr<Destructor<F>> make_shared_destructor(const F &f){
    return std::make_shared<Destructor<F>>(f);
  }
  
}
