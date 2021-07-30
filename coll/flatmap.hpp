#pragma once

#include "base.hpp"
#include "traits.hpp"

#include "foreach.hpp"

namespace coll {
struct FlatmapArgsTag {};

template<typename M>
struct FlatmapArgs {
  using TagType = FlatmapArgsTag;

  M mapper;

  template<typename Input>
  using MapperResultType = typename traits::invocation<M, Input>::result_t;
};

template<typename M>
inline FlatmapArgs<M> flatmap(M mapper) { return {std::forward<M>(mapper)}; }

inline auto flatten() {
  return flatmap([](auto&& e) -> decltype(auto) {
    return std::forward<decltype(e)>(e);
  });
}

template<typename Parent, typename Args>
struct Flatmap {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using MapperResultType = typename Args::template MapperResultType<InputType>;
  constexpr static bool IsIterable = traits::is_iterable<MapperResultType>::value;
  constexpr static bool IsCStr = traits::is_c_str<MapperResultType>::value;
  constexpr static bool IsCollOperator = traits::is_pipe_operator<MapperResultType>::value;

  template<bool, typename T> struct CStrOutputType {
    using type = decltype(*std::declval<T>());
  };
  template<typename T> struct CStrOutputType<false, T> {
    using type = void;
  };
  template<bool, typename T> struct CollOperatorOutputType {
    using type = typename traits::remove_cvr_t<T>::OutputType;
  };
  template<typename T> struct CollOperatorOutputType<false, T> {
    using type = void;
  };
  using OutputType =
    std::conditional_t<IsIterable,     typename traits::iterable<MapperResultType, IsIterable>::element_t,
    std::conditional_t<IsCStr,         typename CStrOutputType<IsCStr, MapperResultType>::type,
    std::conditional_t<IsCollOperator, typename CollOperatorOutputType<IsCollOperator, MapperResultType>::type,
    NullArg
  >>>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;

    template<typename ...X>
    Execution(const Args& args, X&& ... x):
      args(args),
      Child(std::forward<X>(x)...) {
    }

    inline void process(InputType e) {
      using Ctrl = traits::operator_control_t<Child>;
      if constexpr (IsIterable) {
        auto&& iterable = args.mapper(std::forward<InputType>(e));
        if constexpr (Ctrl::is_reversed) {
          for (auto i = std::rbegin(iterable), e = std::rend(iterable);
               i != e && !this->control.break_now; ++i) {
            Child::process(*i);
          }
        } else {
          for (auto i = std::begin(iterable), e = std::end(iterable);
               i != e && !this->control.break_now; ++i) {
            Child::process(*i);
          }
        }
      } else if constexpr (IsCStr) {
        // For c-style string that ends with '\0'.
        auto cstr = args.mapper(std::forward<InputType>(e));
        if constexpr (Ctrl::is_reversed) {
          for (auto e = cstr;; ++e) {
            if (*e) {
              for (auto i = e; i != cstr && !this->control.break_now;) {
                Child::process(*(--i));
              }
              break;
            }
          }
        } else {
          for (auto i = cstr; *i && !this->control.break_now; ++i) {
            Child::process(*i);
          }
        }
      } else /* if constexpr (IsCollOperator) */ {
        args.mapper(std::forward<InputType>(e))
          | foreach(std::bind(&Child::process, this, std::placeholders::_1));
      }
    }
  };

  template<typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<Execution<Child>, Args&, X...>(
      args, std::forward<X>(x)...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, FlatmapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Flatmap<P, A>
operator | (Parent&& parent, Args&& args) {
  static_assert(!std::is_same<typename Flatmap<P, A>::OutputType, NullArg>::value,
    "Return value of lambda of flatmap is not an iterable or a coll operator.");
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
