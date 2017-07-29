
#define LARS_VISITOR_NO_DYNAMIC_CAST
//#define LARS_VISITOR_DEMANGLE_NAMES

#ifdef LARS_VISITOR_NO_DYNAMIC_CAST
#include <unordered_map>
#endif

#ifdef LARS_VISITOR_DEMANGLE_NAMES
#include <lars/demangle.h>
#else
namespace lars{
  namespace{
    inline const char * demangle(const char *name){
      return name;
    }
  }
}
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
  
#pragma mark -
#pragma mark Helper
  namespace visitor_helper{
  
    template <typename ... Args> class TypeList{};

    template <class T,int ORDER> struct OrderedType{ using Type = T; const static int value = ORDER; };
    template <typename ... Args> class OrderedTypeSet{};
    
    template <int ... Args> struct HighestOrder;
    template <int first,int ... Rest> struct HighestOrder<first,Rest...>{ const static int value = HighestOrder<Rest...>::value; };
    template <int last> struct HighestOrder<last>{ const static int value = last; };
    template <> struct HighestOrder<>{ const static int value = 0; };
    
    template <typename ... Args> struct HighestOrderInOrderedTypeSet;
    template <typename ... Args> struct HighestOrderInOrderedTypeSet<OrderedTypeSet<Args...>>{ const static int value = HighestOrder<Args::value...>::value; };
    
    template <int ... Args> struct LowestOrder;
    template <int first,int ... Rest> struct LowestOrder<first,Rest...>{ const static int value = first; };
    template <> struct LowestOrder<>{ const static int value = 0; };
    
    template <typename ... Args> struct LowestOrderInOrderedTypeSet;
    template <typename ... Args> struct LowestOrderInOrderedTypeSet<OrderedTypeSet<Args...>>{ const static int value = LowestOrder<Args::value...>::value; };
    
    
    template <typename ... Args> class Before{};
    
    template <typename ... Args> struct OrderedTypeSetContains;
    template <class First,int FORDER,typename ... Args,class T> struct OrderedTypeSetContains<OrderedTypeSet<OrderedType<First, FORDER>,Args...>,T>:public OrderedTypeSetContains<OrderedTypeSet<Args...>,T>{ };
    template <typename ... Args,class T,int ORDER> struct OrderedTypeSetContains<OrderedTypeSet<OrderedType<T,ORDER>,Args...>,T>:public std::true_type{ };
    template <class T> struct OrderedTypeSetContains<OrderedTypeSet<>,T>:public std::false_type{ };
    
    template <typename ... Args> class AddToOrderedTypeSet;
    template <typename ... Args,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>>{
    public:
      using Type = typename AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>,Before<>>::Type;
    };
    
    template <typename ... Args,typename ... BArgs,class FT,int FORDER,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<OrderedType<FT,FORDER>,Args...>,OrderedType<T,ORDER>,Before<BArgs...>>{
      static const bool insert_before = ORDER < FORDER;
      using Contained = OrderedTypeSetContains<OrderedTypeSet<Args...>,T>;
      
      using IfContained = OrderedTypeSet<BArgs...,OrderedType<FT,FORDER>,Args...> ;
      using IfNotContained = OrderedTypeSet<BArgs...,OrderedType<T,ORDER>,OrderedType<FT,FORDER>,Args...> ;
      
      using If = typename std::conditional<Contained::value,IfContained,IfNotContained>::type;
      using Else = typename AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>,Before<BArgs...,OrderedType<FT,FORDER>>>::Type;
    public:
      using Type = typename std::conditional<insert_before,If,Else>::type;
    };
    
    template <typename ... Args,typename ... BArgs,int FORDER,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<OrderedType<T,FORDER>,Args...>,OrderedType<T,ORDER>,Before<BArgs...>>{
      static const bool insert_before = ORDER < FORDER;
      using If = OrderedTypeSet<BArgs...,OrderedType<T,ORDER>,Args...> ;
      using Else = typename AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>,Before<BArgs...>>::Type;
    public:
      using Type = typename std::conditional<insert_before,If,Else>::type;
    };
    
    template <typename ... BArgs,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<>,OrderedType<T,ORDER>,Before<BArgs...>>{
    public:
      using Type = OrderedTypeSet<BArgs...,OrderedType<T, ORDER>>;
    };
    
    template <class A,class B> struct JoinOrderedTypeSets;
    template <typename ... ArgsA,class ArgsB> struct JoinOrderedTypeSets<OrderedTypeSet<ArgsA...>,OrderedTypeSet<ArgsB>>{
      using Type = typename AddToOrderedTypeSet<OrderedTypeSet<ArgsA...>,ArgsB>::Type;
    };
    
    template <typename ... ArgsA,class First,typename ... ArgsB> struct JoinOrderedTypeSets<OrderedTypeSet<ArgsA...>,OrderedTypeSet<First,ArgsB...>>{
      using Type = typename JoinOrderedTypeSets< typename AddToOrderedTypeSet<OrderedTypeSet<ArgsA...>,First>::Type, OrderedTypeSet<ArgsB...> >::Type;
    };
    
    template <typename ... ArgsA> struct JoinOrderedTypeSets<OrderedTypeSet<ArgsA...>,OrderedTypeSet<>>{
      using Type = OrderedTypeSet<ArgsA...>;
    };
    
    template<typename T> struct to_void { typedef void type; };
    
    template <typename ... Args> struct ExtractBaseTypes;
    
    template <typename T, typename = void> struct AllBaseTypesOfSingleType{ using Type = OrderedTypeSet<>; };
    template <typename T> struct AllBaseTypesOfSingleType <T, typename to_void<typename T::VisitableBaseTypes>::type>{ using Type = typename ExtractBaseTypes<typename T::VisitableBaseTypes>::Type; };
    
    template <typename ... Args> struct AllBaseTypes;
    template <class First,typename ... Args> struct AllBaseTypes<First,Args...>{
      using Type = typename JoinOrderedTypeSets<typename AllBaseTypes<First>::Type, typename AllBaseTypes<Args...>::Type>::Type;
    };
    template <class T> struct AllBaseTypes<T>{ using Type = typename AllBaseTypesOfSingleType<T>::Type; };
    
    template <> struct ExtractBaseTypes<OrderedTypeSet<>>{ using Type = OrderedTypeSet<>; };
    template <class First,typename ... Args> struct ExtractBaseTypes<OrderedTypeSet<First,Args...>>{
      using TypeOfFirst = typename AddToOrderedTypeSet< typename AllBaseTypes<First>::Type, First >::Type;
      using Type = typename JoinOrderedTypeSets<TypeOfFirst,typename ExtractBaseTypes<OrderedTypeSet<Args...>>::Type>::Type;
    };
    
    template <class Stream,class T,int ORDER> Stream &operator<<(Stream &stream,OrderedType<T,ORDER>){
      stream << '(' << lars::demangle(typeid(T).name()) << ',' << ORDER << ')';
      return stream;
    }
    
    template <class Stream,typename ... Args> Stream &operator<<(Stream &stream,OrderedTypeSet<Args...>){
      stream << '{';
      lars::dummy_function(operator<<(stream,Args())...);
      stream << '}';
      return stream;
    }
    
  }
  
#pragma mark -
#pragma mark Predefinitions
  
  class VisitorBase;
  class ConstVisitorBase;
  class RecursiveVisitorBase;
  class RecursiveConstVisitorBase;
  
  template <typename ... Args> class WithBaseClass{};
  template <typename ... Args> class WithVisitableBaseClass{};
  template <typename ... Args> class WithStaticVisitor{};
  
  class VisitableBase;
  
  template <typename ... Args> class Visitable;
  template <typename ... Args> class StaticVisitable;
  template <typename ... Args> class VisitableAndStaticVisitable;
  
#pragma mark -
#pragma mark Visitor
  
  template <typename ... Args> class VisitorPrototype;
  
  template <class Base,typename Visiting,typename ... VisitingRest,class Accepted,typename ... AcceptedRest,class Return> class VisitorPrototype<Base,visitor_helper::TypeList<Visiting,VisitingRest...>,visitor_helper::TypeList<Accepted,AcceptedRest...>,Return>:public virtual Base,public VisitorPrototype<Base,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<Accepted>,Return>,public VisitorPrototype<Base,visitor_helper::TypeList<VisitingRest...>,visitor_helper::TypeList<AcceptedRest...>,Return>{
  protected:
    
    void * try_to_cast_to(const std::type_info &requested){
      auto result = VisitorPrototype<Base,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<Accepted>,Return>::try_to_cast_to(requested);
      if(result != nullptr) return result;
      return VisitorPrototype<Base,visitor_helper::TypeList<VisitingRest...>,visitor_helper::TypeList<AcceptedRest...>,Return>::try_to_cast_to(requested);
    }
    
  public:
    using ConstVisitor = VisitorPrototype<ConstVisitorBase,visitor_helper::TypeList<Visiting,VisitingRest...>,visitor_helper::TypeList<const Visiting &,const VisitingRest &...>,Return>;
    
    void * as_visitor_for(const std::type_info &requested)override{
      return try_to_cast_to(requested);
    }
    
  };
  
  template <class Base,typename Visiting,class Accepted,class Return> class VisitorPrototype<Base,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<Accepted>,Return>:public virtual Base{
  protected:
    
    void * try_to_cast_to(const std::type_info &requested){
      if(typeid(Visiting) == requested){ return reinterpret_cast<void*>(this); }
      return nullptr;
    }
    
  public:
    using ConstVisitor = VisitorPrototype<ConstVisitorBase,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<const Visiting &>,Return>;
    
    void * as_visitor_for(const std::type_info &requested)override{
      return try_to_cast_to(requested);
    }
    
    virtual Return visit(Accepted) = 0;
  };
  
  template <typename ... Args> using Visitor = VisitorPrototype<VisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<Args &...>,void>;
  template <typename ... Args> using ConstVisitor = VisitorPrototype<ConstVisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<const Args &...>,void>;
  template <typename ... Args> using RecursiveVisitor = VisitorPrototype<RecursiveVisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<Args &...>,bool>;
  template <typename ... Args> using RecursiveConstVisitor = VisitorPrototype<RecursiveConstVisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<const Args &...>,bool>;
  
#pragma mark -
#pragma mark Base Classes
  
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
  
  class VisitorBase:public VisitorBasePrototype<Visitor>{};
  class ConstVisitorBase:public VisitorBasePrototype<ConstVisitor>{};
  class RecursiveVisitorBase:public VisitorBasePrototype<RecursiveVisitor>{};
  class RecursiveConstVisitorBase:public VisitorBasePrototype<RecursiveConstVisitor>{};
  
  class VisitableBase{
  public:
    virtual void accept(VisitorBase &visitor) = 0;
    virtual void accept(ConstVisitorBase &visitor)const = 0;
    virtual void accept(RecursiveVisitorBase &visitor) = 0;
    virtual void accept(RecursiveConstVisitorBase &visitor)const = 0;
    virtual ~VisitableBase(){}
  };
  
  
#pragma mark -
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
    
    void accept(RecursiveVisitorBase &visitor)override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<T &>(*this));
      }
    }
    
    void accept(RecursiveConstVisitorBase &visitor)const override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<const T &>(*this));
      }
    }
    
  };
  
  template <class ... Bases,class ... VisitableBases,class OrderedBases> class Visitable<WithBaseClass<Bases...>,WithVisitableBaseClass<VisitableBases...>,OrderedBases>:public virtual VisitableBase,public virtual Bases...{
  private:
    
    struct IncompatibleVisitorException:public lars::IncompatibleVisitorException{};

    template <class Current> void try_to_accept(VisitorBase &visitor){
      VISITOR_LOG("try to accept: " << typeid(Current).name());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<Current &>(*this));
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
        casted->visit(static_cast<const Current &>(*this));
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
    
    template <class Current> void try_to_accept(RecursiveVisitorBase &visitor){
      if(auto casted = visitor.as_visitor_for<Current>()){
        casted->visit(static_cast<Current &>(*this));
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(RecursiveVisitorBase &visitor){
      if(auto casted = visitor.as_visitor_for<Current>()){
        if(!casted->visit(static_cast<Current &>(*this))) return;
      }
      try_to_accept<Second,Rest ...>(visitor);
    }

    template <class Current> void try_to_accept(RecursiveConstVisitorBase &visitor)const{
      if(auto casted = visitor.as_visitor_for<Current>()){
        casted->visit(static_cast<const Current &>(*this));
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(RecursiveConstVisitorBase &visitor)const{
      if(auto casted = visitor.as_visitor_for<Current>()){
        if(!casted->visit(static_cast<const Current &>(*this))) return;
      }
      try_to_accept<Second,Rest ...>(visitor);
    }

  public:
    
    using VisitableBaseTypes = OrderedBases;
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

    void accept(VisitorBase &visitor)override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
    void accept(ConstVisitorBase &visitor)const override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
    void accept(RecursiveVisitorBase &visitor)override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
    void accept(RecursiveConstVisitorBase &visitor)const override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
#pragma clang diagnostic pop

  };
  
  template <typename ... Args> struct ToWithVisitableBaseClass;
  template <typename ... Args> struct ToWithVisitableBaseClass<visitor_helper::OrderedTypeSet<Args...>>{
    using Type = WithVisitableBaseClass<typename Args::Type ...>;
  };
  
#pragma mark -
#pragma mark Derived Visitable
  
#define LARS_REMOVE_VISITABLE_ACCEPT_METHODS() void accept(lars::VisitorBase&)override = 0; void accept(lars::ConstVisitorBase&)const override = 0; void accept(lars::RecursiveVisitorBase&)override = 0; void accept(lars::RecursiveConstVisitorBase&)const override = 0
  
#define LARS_DEFINE_FORWARDED_VISITABLE_ACCEPT_METHODS(BASE) \
  void accept(::lars::VisitorBase &visitor)override{ BASE::accept(visitor); }\
  void accept(::lars::ConstVisitorBase &visitor)const override{ BASE::accept(visitor); }\
  void accept(::lars::RecursiveVisitorBase &visitor)override{ BASE::accept(visitor); }\
  void accept(::lars::RecursiveConstVisitorBase &visitor)const override{ BASE::accept(visitor); }
  
  template <typename ... Args> class DerivedVisitable;
  
  template <typename ... Bases,typename ... VisitableBases> class DerivedVisitable<WithBaseClass<Bases...>,WithVisitableBaseClass<VisitableBases...>>{
    using AllBaseTypes = typename visitor_helper::AllBaseTypes<VisitableBases...>::Type;
    static const int current_order = visitor_helper::LowestOrderInOrderedTypeSet<AllBaseTypes>::value;
  public:
    using BaseTypeList = typename visitor_helper::JoinOrderedTypeSets<AllBaseTypes, visitor_helper::OrderedTypeSet<visitor_helper::OrderedType<VisitableBases,current_order>...> >::Type;
    using Type = Visitable<WithBaseClass<Bases...> , typename ToWithVisitableBaseClass<BaseTypeList>::Type, BaseTypeList>;
  };
  
  template <class T,typename ... Bases,typename ... VisitableBases> class DerivedVisitable<T,WithBaseClass<Bases...>,WithVisitableBaseClass<VisitableBases...>>{
    using AllBaseTypes = typename visitor_helper::AllBaseTypes<VisitableBases...>::Type;
    static const int current_order = visitor_helper::LowestOrderInOrderedTypeSet<AllBaseTypes>::value;
  public:
    using BaseTypeList = typename visitor_helper::JoinOrderedTypeSets<AllBaseTypes, visitor_helper::OrderedTypeSet<visitor_helper::OrderedType<T,current_order-1>,visitor_helper::OrderedType<VisitableBases,current_order>...> >::Type;
    using Type = Visitable<WithBaseClass<Bases...> , typename ToWithVisitableBaseClass<BaseTypeList>::Type, BaseTypeList>;
  };
  
  template <class T,typename ... Bases> class DerivedVisitable<T,WithVisitableBaseClass<Bases...>>:public DerivedVisitable<T,WithBaseClass<Bases...>,WithVisitableBaseClass<Bases...>>{  };
  template <typename ... Bases> class DerivedVisitable<WithVisitableBaseClass<Bases...>>:public DerivedVisitable<WithBaseClass<Bases...>,WithVisitableBaseClass<Bases...>>{  };
  
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
