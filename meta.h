template <class>
struct Resolve;
template <typename ...Types, template <typename ...> class T>
struct Resolve<T<Types...>> {
  using type = T<Types...>;
};
template <typename Type>
struct Resolve {
 private:
  template<class U>
  static typename U::type check(int);
  template <class U>
  static U check(...);
 public:
  using type = decltype(check<Type>(0));
};

/* // Template meta-programming helpers */
/* template<class...Ts>struct types { */
/*   using type=types; // makes inheriting from it useful */
/*   static constexpr std::size_t size = sizeof...(Ts); */
/* }; */
/* template<template<class...>class Z, class List> */
/* struct fmap {}; */
/* template<template<class...>class Z, class List> */
/* using fmap_t = typename fmap<Z,List>::type; */
/* template<template<class...>class Z, class...Ts> */
/* struct fmap<Z, types<Ts...>>: */
/*   types<Z<Ts>...> */
/* {}; */
/* template<template<class...>class Z, class List> */
/* struct fapply {}; */
/* template<template<class...>class Z, class List> */
/* using fapply_t=typename fapply<Z,List>::type; */
/* template<template<class...>class Z, class...Ts> */
/* struct fapply<Z, types<Ts...>> { */
/*   using type=Z<Ts...>; */
/* }; */
/* template<template<class...>class Z> */
/* struct applier { */
/*     template<class List> */
/*     using apply = fapply_t<Z,List>; */
/* }; */
