
#define LARS_VISITOR_NO_DYNAMIC_CAST

#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#endif

#include <exception>

namespace lars{
  
  class VisitableBase;
  template <typename ... Args> class Visitor;
  template <typename ... Args> class Visitable;

  class VisitorBase{
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
    virtual ~VisitorBase(){}
  };

  class VisitableBase{
  public:
    virtual void accept(VisitorBase &visitor) = 0;
    virtual ~VisitableBase(){}
  };
  
  template <typename First,typename Second,typename ... Rest> class Visitor<First,Second,Rest...>:public Visitor<First>,public Visitor<Second,Rest...>{
  public:
  };
  
  template <typename T> class Visitor<T>:public virtual VisitorBase{
  public:
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
    Visitor(){
      VisitorBase::derived_types.emplace(typeid(Visitor<T>),reinterpret_cast<void*>(this));
    }
#endif
    virtual void visit(T &) = 0;
  };
  
  template <> class Visitor<>:public VisitableBase{
  protected:
    static void visit(){}
  };
  
  struct IncompatibleVisitorException:public std::exception{};
  
  template <class T> class Visitable<T>:public virtual VisitableBase{
  public:
    
    struct IncompatibleVisitorException:public lars::IncompatibleVisitorException{};
    
    void accept(VisitorBase &visitor)override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<T &>(*this));
      }
      else{
        throw IncompatibleVisitorException();
      }
    }
  };
  
  template <class T,class ... Bases> class Visitable<T,Bases...>:public virtual Bases ...{
  private:

    template <class Current> void try_to_accept(VisitorBase &visitor){
      Current::accept(visitor);
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(VisitorBase &visitor){
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<T &>(*this));
        return;
      }
#else
      if(auto * casted = dynamic_cast<Visitor<Current>*>(&visitor)){
        casted->visit(static_cast<Current &>(*this));
        return;
      }
#endif
      try_to_accept<Second,Rest ...>(visitor);
    }
    
  public:
    void accept(VisitorBase &visitor)override{
      try_to_accept<T, Bases...>(visitor);
    }
  };

  
}
