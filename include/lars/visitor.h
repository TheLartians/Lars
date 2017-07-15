
#include <stdexcept>

namespace lars{
  
  class VisitableBase;
  
  class VisitorBase{
  public:
    virtual ~VisitorBase(){}
  };

  class VisitableBase{
  public:
    virtual void accept(VisitorBase &visitor) = 0;
    virtual ~VisitableBase(){}
  };
  
  template <typename ... Args> class Visitor;
  template <class T> class Visitable;
  
  template <typename First,typename Second,typename ... Rest> class Visitor<First,Second,Rest...>:public Visitor<First>,public Visitor<Second,Rest...>{
  public:
  };
  
  template <typename T> class Visitor<T>:public virtual VisitorBase{
  public:
    virtual void visit(T &) = 0;
  };
  
  template <> class Visitor<>:public VisitableBase{
  protected:
    static void visit(){}
  };
  
  template <class T> class Visitable:public virtual VisitableBase{
  public:
    void accept(VisitorBase &visitor)override{
      dynamic_cast<Visitor<T>&>(visitor).visit(static_cast<T &>(*this));
    }
  };
  
  template <class T,class Base> class RecursiveVisitable:public virtual Base{
  public:
    void accept(VisitorBase &visitor)override{
      if(auto * casted = dynamic_cast<Visitor<T>*>(&visitor)){
        casted->visit(static_cast<T &>(*this));
      }
      else{
        Base::accept(visitor);
      }
    }
  };
  
  template <class T,class ... Bases> class IterativeVisitable:public virtual Bases ...{
  private:

    template <class Current> void try_to_accept(VisitorBase &visitor){
      Current::accept(visitor);
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(VisitorBase &visitor){
      if(auto * casted = dynamic_cast<Visitor<Current>*>(&visitor)){
        casted->visit(static_cast<Current &>(*this));
        return;
      }
      try_to_accept<Second,Rest ...>(visitor);
    }
    
  public:
    void accept(VisitorBase &visitor)override{
      try_to_accept<T, Bases...>(visitor);
    }
  };

  
}
