
#define LARS_VISITOR_NO_DYNAMIC_CAST

#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#endif

#ifndef LARS_VISITOR_NO_EXCEPTIONS
#include <exception>
#endif

namespace lars{
  
  class VisitableBase;
  template <typename ... Args> class Visitor;
  template <typename ... Args> class ConstVisitor;
  template <typename ... Args> class Visitable;

  template <template <typename ... Args> class Visitor> class VisitorBasePrototype{
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
  protected:
    std::unordered_map<std::type_index,void *> derived_types;
  public:
    template <class T> Visitor<T> * as_visitor_for(){
      auto it = derived_types.find(typeid(Visitor<T>));
      if(it == derived_types.end()) return nullptr;
      return reinterpret_cast<Visitor<T>*>(it->second);
    }
#else
  public:
    template <class T> Visitor<T> * as_visitor_for(){
      return dynamic_cast<Visitor<T> *>(this);
    }
#endif
    virtual ~VisitorBasePrototype(){}
  };
  
  using VisitorBase = VisitorBasePrototype<Visitor>;
  using ConstVisitorBase = VisitorBasePrototype<ConstVisitor>;;
  
  class VisitableBase{
  public:
    virtual void accept(VisitorBase &visitor) = 0;
    virtual void accept(ConstVisitorBase &visitor)const = 0;
    virtual ~VisitableBase(){}
  };
  
  template <typename First,typename Second,typename ... Rest> class Visitor<First,Second,Rest...>:public Visitor<First>,public Visitor<Second,Rest...>{
  public:
    using ConstVisitor = lars::ConstVisitor<First,Second,Rest...>;
  };
  
  template <typename T> class Visitor<T>:public virtual VisitorBase{
  public:
    using ConstVisitor = lars::ConstVisitor<T>;

#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
    Visitor(){
      VisitorBase::derived_types.emplace(typeid(Visitor<T>),reinterpret_cast<void*>(this));
    }
#endif
    virtual void visit(T &) = 0;
  };
  
  template <typename First,typename Second,typename ... Rest> class ConstVisitor<First,Second,Rest...>:public ConstVisitor<First>,public ConstVisitor<Second,Rest...>{
  public:
  };
  
  template <typename T> class ConstVisitor<T>:public virtual ConstVisitorBase{
  public:
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
    ConstVisitor(){
      ConstVisitorBase::derived_types.emplace(typeid(ConstVisitor<T>),reinterpret_cast<void*>(this));
    }
#endif
    virtual void visit(const T &) = 0;
  };
  
#ifndef LARS_VISITOR_NO_EXCEPTIONS
  struct IncompatibleVisitorException:public std::exception{};
#endif
  
  template <class T> class Visitable<T>:public virtual VisitableBase{
  public:
    
#ifndef LARS_VISITOR_NO_EXCEPTIONS
    struct IncompatibleVisitorException:public lars::IncompatibleVisitorException{};
#endif
    
    void accept(VisitorBase &visitor)override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<T &>(*this));
      }
      else{
#ifndef LARS_VISITOR_NO_EXCEPTIONS
        throw IncompatibleVisitorException();
#endif
      }
    }
    
    void accept(ConstVisitorBase &visitor)const override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<const T &>(*this));
      }
      else{
#ifndef LARS_VISITOR_NO_EXCEPTIONS
        throw IncompatibleVisitorException();
#endif
      }
    }
    
  };
  
  template <class T,class ... Bases> class Visitable<T,Bases...>:public virtual Bases ...{
  private:

    template <class Current> void try_to_accept(VisitorBase &visitor){
      Current::accept(visitor);
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(VisitorBase &visitor){
      if(auto casted = visitor.as_visitor_for<Current>()){
        casted->visit(static_cast<Current &>(*this));
        return;
      }
      try_to_accept<Second,Rest ...>(visitor);
    }

    template <class Current> void try_to_accept(ConstVisitorBase &visitor)const{
      Current::accept(visitor);
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(ConstVisitorBase &visitor)const{
      if(auto casted = visitor.as_visitor_for<Current>()){
        casted->visit(static_cast<const Current &>(*this));
        return;
      }
      try_to_accept<Second,Rest ...>(visitor);
    }
    
  public:
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

    void accept(VisitorBase &visitor)override{
      try_to_accept<T, Bases...>(visitor);
    }
    
    void accept(ConstVisitorBase &visitor)const override{
      try_to_accept<T, Bases...>(visitor);
    }
    
#pragma clang diagnostic pop

  };
  
#define LARS_MAKE_STATIC_VISITABLE(VISITOR) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Woverloaded-virtual\"") _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") virtual void accept(VISITOR &visitor){ visitor.visit(*this); } _Pragma("clang diagnostic pop")
#define LARS_MAKE_STATIC_CONST_VISITABLE(VISITOR) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Woverloaded-virtual\"") _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") virtual void accept(VISITOR &visitor)const{ visitor.visit(*this); } _Pragma("clang diagnostic pop")
#define LARS_MAKE_STATIC_VISITABLE_AND_CONST_VISITABLE(VISITOR) LARS_MAKE_STATIC_VISITABLE(VISITOR) LARS_MAKE_STATIC_CONST_VISITABLE(VISITOR::ConstVisitor)
  
  template <typename ... Args> class StaticVisitor;
  template <typename ... Args> class ConstStaticVisitor;
  
  template <class First,typename ... Rest> class StaticVisitor<First,Rest...>:public StaticVisitor<Rest...>{
  public:
    using ConstVisitor = lars::ConstStaticVisitor<First,Rest...>;
    using StaticVisitor<Rest...>::visit;
    virtual void visit(First &) = 0;
  };

  
}
