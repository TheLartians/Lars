#pragma once

#define MUTATOR_MEMBER(TYPE,NAME,DEFAULT,CODE) private: TYPE _ ## NAME DEFAULT; public: TYPE const &NAME()const{ return _ ## NAME; } void set_ ## NAME(TYPE const &NAME){ _ ## NAME = NAME; CODE; }

