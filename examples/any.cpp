#include <iostream>
#include <lars/any.h>

using namespace lars;

int main(){
  Any a = make_any<std::shared_ptr<std::string>>(std::make_shared<std::string>("test"));
  std::cout << *a.get<std::shared_ptr<std::string>>() << std::endl;
  std::cout << a.get<std::string>() << std::endl;
  
  Any b = make_any<float>(42.42);
  std::cout << b.get<int>() << std::endl;
  std::cout << b.get<double>() << std::endl;
  
  struct Base:Visitable<Base>{int a;};
  struct Derived:public DVisitable<Derived, Base>{Derived(int v){ b=v; a=v+1; } int b;};

  Any c = make_any<Derived>(42);
  std::cout << c.get<Base>().a << std::endl;
  std::cout << c.get<std::shared_ptr<Base>>()->a << std::endl;
  std::cout << c.get<std::shared_ptr<Derived>>()->b << std::endl;
  std::cout << c.get<Derived>().b << std::endl;

  Any d = make_any<std::shared_ptr<Derived>>(std::make_shared<Derived>(45));
  std::cout << d.get<Base>().a << std::endl;
  std::cout << d.get<std::shared_ptr<Base>>()->a << std::endl;
  std::cout << d.get<std::shared_ptr<Derived>>()->b << std::endl;
  std::cout << d.get<Derived>().b << std::endl;
  
  lars::AnyFunction f = [](Derived &d){ std::cout << d.b << std::endl; };
  f(d);
  f(d.get<Derived>());
  lars::AnyFunction g = [](const Base &d){ std::cout << d.a << std::endl; };
  g(d);
  g(d.get<Derived>());
  g(d.get<Base>());
  lars::AnyFunction h = [](const std::shared_ptr<Derived> &d){ std::cout << d->a << std::endl; };
  h(d);
}
