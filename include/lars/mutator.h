#pragma once

#define MUTATOR_MEMBER(TYPE,NAME,DEFAULT,CODE) private: TYPE _ ## NAME DEFAULT; public: const TYPE &NAME()const{ return _ ## NAME; } void set_ ## NAME(const TYPE &NAME){ _ ## NAME = NAME; CODE; }

