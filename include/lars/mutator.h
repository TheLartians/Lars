#pragma once

#define MUTATOR_MEMBER(TYPE,NAME,DEFAULT,CODE) private: TYPE _ ## NAME DEFAULT; public: TYPE const &NAME()const{ return _ ## NAME; } void set_ ## NAME(TYPE const & __ ## NAME){ _ ## NAME = __ ## NAME; CODE; }

#define MUTATOR_MEMBER_VISIBILITY(TYPE,NAME,DEFAULT,CODE,GETTER_VISIBILITY,SETTER_VISIBILITY) private: TYPE _ ## NAME DEFAULT; GETTER_VISIBILITY TYPE const &NAME()const{ return _ ## NAME; } SETTER_VISIBILITY void set_ ## NAME(TYPE const &__ ## NAME){ _ ## NAME = __ ## NAME; CODE; }

