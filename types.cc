#include <functional>
#include <tuple>
#include <type_traits>
#include <variant>

// SFINAE to check if the first type is a template instance of the second type
template <class, template <class...> class>
struct is_template_instance : public std::false_type {};

// This specialization is chosen iff the both template arguments are class U
// and the first class is U<Ts...>
template <class...Ts, template <class...> class U>
struct is_template_instance<U<Ts...>, U> : public std::true_type {};

template <template <class...> class U, class V>
struct is_template_instance<typename V::Base, U> {};



// Template meta-programming helpers
template<class...Ts>struct types {
  using type=types; // makes inheriting from it useful
  static constexpr std::size_t size = sizeof...(Ts);
};
template<template<class...>class Z, class List>
struct fmap {};
template<template<class...>class Z, class List>
using fmap_t = typename fmap<Z,List>::type;
template<template<class...>class Z, class...Ts>
struct fmap<Z, types<Ts...>>:
  types<Z<Ts>...>
{};
template<template<class...>class Z, class List>
struct fapply {};
template<template<class...>class Z, class List>
using fapply_t=typename fapply<Z,List>::type;
template<template<class...>class Z, class...Ts>
struct fapply<Z, types<Ts...>> {
  using type=Z<Ts...>;
};
template<template<class...>class Z>
struct applier {
    template<class List>
    using apply = fapply_t<Z,List>;
};


template <typename ...DependencyLists>
struct Node;

struct LiteralNode;

/* template <typename T> */
/* struct IsNode { */
/*     static const bool value = std::disjunction_v< */
/*         is_template_instance<T, Node>, */
/*         std::is_base_of<LiteralNode, T> */
/*     >; */
/* }; */

template <typename T, typename ...Ts>
struct ContainsT : public std::disjunction<std::is_same<T, Ts>...> {};

// Each member of Dependencies should be a BaseNode, either referential or literal
template <typename ...Dependencies>
struct DependencyList {
    static_assert(
        std::conjunction_v<
            std::disjunction<
                is_template_instance<Dependencies, Node>,
                std::is_same<LiteralNode, Dependencies>
            >...
        >,
        "Each member of DependencyLists should be a DependencyList"
    );
    DependencyList() {}
    DependencyList(Dependencies ...dep)
        : children({dep...}) {}

    std::tuple<Dependencies...> children;
};

template <typename ...>
struct AL_DLE {};

template <typename ...Deps, typename ...Nodes>
struct AL_DLE<DependencyList<Deps...>, Nodes...> : public std::conjunction<
    ContainsT<Deps, LiteralNode, Nodes...>...
> {};

template <typename ...DependencyLists>
struct Node {
    static_assert(
        std::conjunction_v<is_template_instance<DependencyLists, DependencyList>...>,
        "Each member of DependencyLists should be a DependencyList"
    );
    using v = std::variant<DependencyLists...>;
    v children;
    static constexpr std::size_t size = sizeof...(DependencyLists);
    template <std::size_t index>
    using dl = std::variant_alternative_t<index, v>;
};

template <typename ...>
struct N_DLE {};

template <typename ...DLs, typename ...Nodes>
struct N_DLE<Node<DLs...>, Nodes...> : public std::conjunction<
    AL_DLE<DLs, Nodes...>...
> {};

struct LiteralNode {};

template <typename ...Nodes>
struct Tree {
    static_assert(
        std::conjunction_v<is_template_instance<Nodes, Node>...>,
        "Each member of Nodes should be a Node"
    );
    static_assert(
        std::conjunction_v<
            N_DLE<Nodes, Nodes...>...
        >,
        "..."
    );
};


// TODO: A transform is associated with a specific DependencyList for a
// specific Node. The arguments to the transform should be the return value
// from the transforms of each Node in the DependencyList that are in the same
// TransformSet.
template <typename Node, typename Return, int index, typename ...Args>
struct Transform {
    static_assert(index < Node::size, "index should be less than Node::size");
    static_assert(sizeof...(Args) == Node::size, "...");
    std::function<Return(Args...)> func;

    template <std::size_t i>
    using arg = std::tuple_element<i, std::tuple<Args...>>;
};

template <typename ...>
struct Matches;

template <>
struct Matches<> : std::true_type {};

template <typename ...>
struct MatchingArg;

template <>
struct MatchingArg<> : std::true_type {};

template <typename ...>
struct MatchingTransform;

template <typename Node, typename Return, int index, typename ...Args, typename ...Transforms>
struct MatchingTransform<Transform<Node, Return, index, Args...>, Transforms...> : std::true_type {};

template <typename Tree, typename ...Transforms>
struct TransformSet {
    static_assert(std::conjunction_v<MatchingTransform<Transforms, Transforms...>...>, "...");
    Tree t;
    std::tuple<Transforms...> transforms;
};

/* struct NodeTT; */
/* struct NodeTT : Node<DependencyList<LiteralNode>,DependencyList<NodeTT>> {}; */

int main() {
    using NodeT = Node<DependencyList<LiteralNode>>;
    using DependencyListT = DependencyList<LiteralNode, NodeT>;
    using DoubleNodeT = Node<DependencyList<LiteralNode, LiteralNode>>;
    using TreeT = Tree<
        NodeT,
        Node<
            DependencyListT
        >,
        Node<
            DependencyList<
                DoubleNodeT
            >
        >,
        DoubleNodeT
        /* , NodeTT */
    >;

    // These should not work
    /* Node<A> b; */
    /* Node<DependencyList<int>> c; */
    /* using TreeT = Tree< */
    /*     NodeT, */
    /*     Node< */
    /*         DependencyListT */
    /*     >, */
    /*     Node< */
    /*         DependencyList< */
    /*             DoubleNodeT */
    /*         > */
    /*     >, */
    /*     // DoubleNodeT  // this is required */
    /* >; */
    /* Tree<int> x; */
    /* static_assert(DependencyListT::AllDependenciesExist<int>::value, "wat"); */

    // These should work
    LiteralNode a;
    DependencyListT d;
    NodeT e;
    TreeT t;
    /* static_assert(DependencyListT::AllDependenciesExist<LiteralNode, NodeT>::value, "wat"); */
    return 0;
}
