
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
  
  template <class T,class Base> class InheritedVisitable:public virtual Base{
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
  
}
