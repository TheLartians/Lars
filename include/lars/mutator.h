#pragma once

namespace lars{
  namespace mutator_helper{
    template <class T,typename ... Args> T initialize(const Args & ... args){
      return T(args...);
    }
  }
}
  
#define MUTATOR_MEMBER_VISIBILITY(TYPE,NAME,DEFAULT,CODE,GETTER_VISIBILITY,SETTER_VISIBILITY) private: TYPE _ ## NAME DEFAULT; GETTER_VISIBILITY TYPE const &NAME()const{ return _ ## NAME; } SETTER_VISIBILITY template <typename ... Args> auto &set_ ## NAME(const Args & ... args){ _ ## NAME = ::lars::mutator_helper::initialize<TYPE>(args...); CODE; return *this; }


#define MUTATOR_MEMBER_PRIVATE(TYPE,NAME,DEFAULT,CODE) MUTATOR_MEMBER_VISIBILITY(TYPE,NAME,DEFAULT,CODE,public:,private:);
#define MUTATOR_MEMBER_PROTECTED(TYPE,NAME,DEFAULT,CODE) MUTATOR_MEMBER_VISIBILITY(TYPE,NAME,DEFAULT,CODE,public:,protected:);
#define MUTATOR_MEMBER_PUBLIC(TYPE,NAME,DEFAULT,CODE) MUTATOR_MEMBER_VISIBILITY(TYPE,NAME,DEFAULT,CODE,public:,public:);

#define MUTATOR_MEMBER(TYPE,NAME,DEFAULT,CODE) MUTATOR_MEMBER_PUBLIC(TYPE,NAME,DEFAULT,CODE);

