#pragma once

#include <iostream>
#include <string>

#include "base.hpp"

namespace coll {

template<typename O, typename F = NullArg>
struct PrintArgs {
  constexpr static std::string_view name = "print";

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

  struct PrintProc {
    PrintProc(const Args& args): args(args) {}

    Args args;
    auto_val(printed, false);
    auto_val(control, default_control());

    inline void process(InputType e) {
      if (likely(printed)) {
        args.out << args.delimiter;
      } else {
        args.out << args.start;
        printed = true;
      }
      if constexpr (Args::has_formatter) {
        args.formater(args.out, std::forward<InputType>(e));
      } else {
        args.out << e;
      }
    }

    inline void end() {
      if (!printed) {
        args.out << args.start;
      }
      args.out << args.end;
    }

    constexpr static ExecutionType execution_type = RunExecution;
    template<typename Exec, typename ... ArgT>
    static void execution(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.process();
      exec.end();
    }
  };

  inline decltype(auto) print() {
    return parent.template wrap<PrintProc, Args&>(args);
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

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "print">* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args args) {
  return Print<Parent, Args>{
    std::forward<Parent>(parent),
    std::forward<Args>(args)
  }.print();
}
} // namespace coll
