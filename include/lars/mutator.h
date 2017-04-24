#pragma once

#define MUTATOR_VARIABLE(TYPE,NAME,DEFAULT,CODE) private: TYPE _ ## NAME DEFAULT; public: const TYPE &NAME(){ return _ ## NAME; } void set_ ## NAME(const TYPE &NAME){ _ ## NAME = NAME; CODE; }

