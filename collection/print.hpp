#pragma once

#include <iostream>
#include <string>

#include "base.hpp"

namespace coll {

template<typename O, typename F = NullArg>
struct PrintArgs {
  O& out;
  std::string start;
  std::string delimiter;
  std::string end;
  F formater;

  constexpr static bool has_formatter =
    !std::is_same<F, NullArg>::value;

  template<typename AnotherO>
  inline PrintArgs<AnotherO, F> to(AnotherO& another) {
    return {
      another,
      std::forward<std::string>(start),
      std::forward<std::string>(delimiter),
      std::forward<std::string>(end),
      std::forward<F>(formater)
    };
  }

  template<typename AnotherF>
  inline PrintArgs<O, AnotherF> format(AnotherF another) {
    return {
      out,
      std::forward<std::string>(start),
      std::forward<std::string>(delimiter),
      std::forward<std::string>(end),
      std::forward<AnotherF>(another)
    };
  }
};

template<typename Parent, typename Args>
struct Print {
  using InputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  inline void foreach() {
    auto ctrl = default_control();
    bool printed = false;
    parent.foreach(ctrl, 
      [&](InputType elem) {
        if (likely(printed)) {
          args.out << args.delimiter;
        } else {
          args.out << args.start;
          printed = true;
        }
        if constexpr (Args::has_formatter) {
          args.formater(args.out, std::forward<InputType>(elem));
        } else {
          args.out << elem;
        }
      });
    if (!printed) {
      args.out << args.start;
    }
    args.out << args.end;
  }
};

inline PrintArgs<std::ostream> println() {
  return {std::cout, "", "\n", "\n"};
}

inline PrintArgs<std::ostream> print(const std::string& delimiter = ", ") {
  return {std::cout, "[", delimiter, "]\n"};
}

inline PrintArgs<std::ostream> print(const std::string& start, const std::string& delimiter, const std::string& end) {
  return {std::cout, start, delimiter, end};
}

template<typename Parent, typename O, typename F,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline void operator | (const Parent& parent, PrintArgs<O, F> args) {
  Print<Parent, PrintArgs<O, F>>{parent, std::move(args)}.foreach();
}
} // namespace coll
