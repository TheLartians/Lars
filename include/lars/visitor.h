
#define LARS_VISITOR_NO_DYNAMIC_CAST

#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
#include <unordered_map>
#endif

#include <typeinfo>
#include <exception>

#include <lars/dummy.h>

// #define VISITOR_DEBUG
#ifdef VISITOR_DEBUG
#include <iostream>
#define VISITOR_LOG(X) { std::cout << "visitor: " << this << ": " << X << std::endl; }
#else
#define VISITOR_LOG(X)
#endif


namespace lars{
  
  namespace visitor_helper{
  
    template <typename ... Args> class TypeList{ };
    
    template <typename ... Args> struct TypelistContains;
    template <class First,typename ... Args,class T> struct TypelistContains<TypeList<First,Args...>,T>:public TypelistContains<TypeList<Args...>,T>{ };
    template <typename ... Args,class T> struct TypelistContains<TypeList<T,Args...>,T>:public std::true_type{ };
    template <class T> struct TypelistContains<TypeList<>,T>:public std::false_type{ };
    
    template <typename ... Args> struct PrependToTypeList;
    template <typename ... Args> struct PrependToTypeList<TypeList<Args...>>{ using Type = TypeList<Args...>; };
    template <typename ... Args,class T> struct PrependToTypeList<TypeList<Args...>,T>{ using Type = typename std::conditional<TypelistContains<TypeList<Args...>,T>::value, TypeList<Args...>, TypeList<T,Args...>>::type; };
    template <typename ... Args,class First,typename ... Rest> struct PrependToTypeList<TypeList<Args...>,First,Rest...>{
      using IfContains = typename PrependToTypeList<TypeList<Args...>,Rest...>::Type;
      using IfDoesNotContain = typename PrependToTypeList<TypeList<First,Args...>,Rest...>::Type;
      using Type = typename std::conditional<TypelistContains<TypeList<Args...>,First>::value, IfContains, IfDoesNotContain>::type;
    };
    
    template <class A,class B> struct JoinTypeLists;
    template <typename ... ArgsA,typename ... ArgsB> struct JoinTypeLists<TypeList<ArgsA...>,TypeList<ArgsB...>>{ using Type = typename PrependToTypeList<TypeList<ArgsA...>,ArgsB...>::Type; };
    
    template<typename T> struct to_void { typedef void type; };
    
    template <typename ... Args> struct ExtractBaseTypes;
    
    template <typename T, typename = void> struct AllBaseTypesOfSingleType{ using Type = TypeList<>; };
    template <typename T> struct AllBaseTypesOfSingleType <T, typename to_void<typename T::VisitableBaseTypes>::type>{ using Type = typename ExtractBaseTypes<typename T::VisitableBaseTypes>::Type; };
    
    template <typename ... Args> struct AllBaseTypes;
    template <class First,typename ... Args> struct AllBaseTypes<First,Args...>{
      using Type = typename JoinTypeLists<typename AllBaseTypes<First>::Type, typename AllBaseTypes<Args...>::Type>::Type;
    };
    template <class T> struct AllBaseTypes<T>{ using Type = typename AllBaseTypesOfSingleType<T>::Type; };
    
    template <> struct ExtractBaseTypes<TypeList<>>{ using Type = TypeList<>; };
    template <class First,typename ... Args> struct ExtractBaseTypes<TypeList<First,Args...>>{
      using TypeOfFirst = typename PrependToTypeList< typename AllBaseTypes<First>::Type, First >::Type;
      using Type = typename JoinTypeLists<TypeOfFirst,typename ExtractBaseTypes<TypeList<Args...>>::Type>::Type;
    };
    
    template <typename Stream,typename ... Args> void operator<<(Stream &stream,TypeList<Args...>){
      stream << '{';
      lars::dummy_function(print_type_list_type(stream,TypeList<Args>())...);
      stream << '}';
    }
    
    template <typename Stream,class T> int print_type_list_type(Stream &stream,TypeList<T>){
      stream << typeid(T).name() << ", ";
      return 0;
    }
    
  }
  
  
  template <typename ... Args> class WithVisitableBaseClass{};
  template <typename ... Args> class WithStaticVisitor{};
  
  class VisitableBase;
  template <typename ... Args> class Visitor;
  template <typename ... Args> class ConstVisitor;
  template <typename ... Args> class Visitable;
  template <typename ... Args> class StaticVisitable;
  template <typename ... Args> class VisitableAndStaticVisitable;

#pragma mark base classes
  
  template <template <typename ... Args> class Visitor> class VisitorBasePrototype{
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
  protected:
    virtual void * as_visitor_for(const std::type_info &) = 0;
  public:
    template <class T> Visitor<T> * as_visitor_for(){
      return reinterpret_cast<Visitor<T> *>(as_visitor_for(typeid(T)));
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
  
#pragma mark Visitor
  
  template <typename First,typename ... Rest> class Visitor<First,Rest...>:public Visitor<First>,public Visitor<Rest...>{
  public:
    using ConstVisitor = lars::ConstVisitor<First,Rest...>;
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
  protected:
    
    void * try_to_cast_to(const std::type_info &requested){
      if(typeid(First) == requested){ return reinterpret_cast<void*>(static_cast<Visitor<First>*>(this)); }
      return Visitor<Rest...>::try_to_cast_to(requested);
    }
    
  public:
    void * as_visitor_for(const std::type_info &requested)override{
      return try_to_cast_to(requested);
    }
#endif
  };
  
  template <typename T> class Visitor<T>:public virtual VisitorBase{
  public:
    using ConstVisitor = lars::ConstVisitor<T>;

#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
  protected:
    
    void * try_to_cast_to(const std::type_info &requested){
      if(typeid(T) == requested){ return reinterpret_cast<void*>(static_cast<Visitor<T>*>(this)); }
      return nullptr;
    }
    
  public:
    
    void * as_visitor_for(const std::type_info &requested)override{
      return try_to_cast_to(requested);
    }
#endif
    virtual void visit(T &) = 0;
  };
  
  template <typename First,typename ... Rest> class ConstVisitor<First,Rest...>:public ConstVisitor<First>,public ConstVisitor<Rest...>{
  public:
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
  protected:
    
    void * try_to_cast_to(const std::type_info &requested){
      if(typeid(First) == requested){ return reinterpret_cast<void*>(static_cast<ConstVisitor<First>*>(this)); }
      return ConstVisitor<Rest...>::try_to_cast_to(requested);
    }
    
  public:
    
    void * as_visitor_for(const std::type_info &requested)override{
      return try_to_cast_to(requested);
    }
#endif
  };
  
  template <typename T> class ConstVisitor<T>:public virtual ConstVisitorBase{
#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
  protected:
    
    void * try_to_cast_to(const std::type_info &requested){
      if(typeid(T) == requested){ return reinterpret_cast<void*>(static_cast<ConstVisitor<T>*>(this)); }
      return nullptr;
    }
    
  public:
    
    void * as_visitor_for(const std::type_info &requested)override{
      return try_to_cast_to(requested);
    }
#endif
  public:
    virtual void visit(const T &) = 0;
  };
  
#pragma mark Visitable
  
#ifndef LARS_VISITOR_NO_EXCEPTIONS
  struct IncompatibleVisitorException:public std::exception{};
#endif
  
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
    
    void accept(ConstVisitorBase &visitor)const override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<const T &>(*this));
      }
      else{
        throw IncompatibleVisitorException();
      }
    }
    
  };
  
  template <class T,class ... Bases> class Visitable<T,WithVisitableBaseClass<Bases...>>:public virtual Bases...,public virtual VisitableBase{
  private:
    
    struct IncompatibleVisitorException:public lars::IncompatibleVisitorException{};

    template <class Current> void try_to_accept(VisitorBase &visitor){
      VISITOR_LOG("try to accept: " << typeid(Current).name());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<T &>(*this));
      }
      else{
        throw IncompatibleVisitorException();
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(VisitorBase &visitor){
      VISITOR_LOG("try to accept: " << typeid(Current).name());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<Current &>(*this));
        return;
      }
      VISITOR_LOG("Continue with: " << typeid(Second).name());
      try_to_accept<Second,Rest ...>(visitor);
    }

    template <class Current> void try_to_accept(ConstVisitorBase &visitor)const{
      VISITOR_LOG("try to accept: " << typeid(Current).name());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<const T &>(*this));
      }
      else{
        throw IncompatibleVisitorException();
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(ConstVisitorBase &visitor)const{
      VISITOR_LOG("try to accept: " << typeid(Current).name());
      if(auto casted = visitor.as_visitor_for<Current>()){
        casted->visit(static_cast<const Current &>(*this));
        VISITOR_LOG("Success!");
        return;
      }
      VISITOR_LOG("Continue with: " << typeid(Second).name());
      try_to_accept<Second,Rest ...>(visitor);
    }
    
  public:
    
    using VisitableBaseTypes = visitor_helper::TypeList<Bases...>;
    
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
  
  template <typename ... Args> struct ToWithVisitableBaseClass;
  template <typename ... Args> struct ToWithVisitableBaseClass<visitor_helper::TypeList<Args...>>{
    using Type = WithVisitableBaseClass<Args...>;
  };
  
  template <class T,class B> class DerivedVisitable;
  template <class T,typename ... Args> class DerivedVisitable<T,WithVisitableBaseClass<Args...>>{
  public:
    using BaseTypeList = typename visitor_helper::PrependToTypeList<typename visitor_helper::AllBaseTypes<Args...>::Type,Args...>::Type;
    using Type = Visitable<T, typename ToWithVisitableBaseClass<BaseTypeList>::Type >;
  };
  
#define LARS_MAKE_STATIC_VISITABLE(VISITOR) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Woverloaded-virtual\"") _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") virtual void static_accept(VISITOR &visitor){ visitor.visit(*this); } _Pragma("clang diagnostic pop")
#define LARS_MAKE_STATIC_CONST_VISITABLE(VISITOR) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Woverloaded-virtual\"") _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") virtual void static_accept(VISITOR &visitor)const{ visitor.visit(*this); } _Pragma("clang diagnostic pop")
#define LARS_MAKE_STATIC_VISITABLE_AND_CONST_VISITABLE(VISITOR) LARS_MAKE_STATIC_VISITABLE(VISITOR) LARS_MAKE_STATIC_CONST_VISITABLE(VISITOR::ConstVisitor)
  
   
  template <typename ... Args> class StaticVisitor;
  
  template <class First,typename ... Rest> class StaticVisitor<First,Rest...>:public StaticVisitor<Rest...>{
  public:
    using ConstVisitor = lars::StaticVisitor<const First,const Rest...>;
    using StaticVisitor<Rest...>::visit;
    virtual void visit(First &) = 0;
  };

  template <> class StaticVisitor<>{
  protected:
    static void visit();
  };
  
  template <typename ... Args> class ForwardVisitsTo;
  
  template <class Visitor,class Target,class First,typename ... Rest> class ForwardVisitsTo<Visitor,Target,First,Rest...>:public virtual Visitor,public ForwardVisitsTo<Visitor,Target,Rest...>{
  public:
    using ConstVisitor = ForwardVisitsTo<typename Visitor::ConstVisitor,const Target,const First,const Rest...>;
    using Visitor::visit;
    void visit(First &visitable)override{ visit((Target &)visitable); }
  };

  template <class Visitor,class Target> class ForwardVisitsTo<Visitor,Target>{};
  
  template <typename ... Args> class RemoveVisitors;
  
  template <class Visitor,class First,typename ... Rest> class RemoveVisitors<Visitor,First,Rest...>:public virtual Visitor,public RemoveVisitors<Visitor,Rest...>{
  public:
    using ConstVisitor = RemoveVisitors<typename Visitor::ConstVisitor,const First,const Rest...>;
    using Visitor::visit;
    void visit(First &visitable)override = 0;
  };
  
  template <class Visitor> class RemoveVisitors<Visitor>{};


  
}
