#pragma once

#include <iostream>
#include <string>

#include "base.hpp"

namespace coll {
struct PrintArgsTag {};

template<typename O, typename F = NullArg>
struct PrintArgs {
  using TagType = PrintArgsTag;

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

inline PrintArgs<std::ostream> println() {
  return {std::cout, "", "\n", "\n"};
}

inline PrintArgs<std::ostream> print(const std::string& delimiter = ", ") {
  return {std::cout, "[", delimiter, "]\n"};
}

inline PrintArgs<std::ostream> print(const std::string& start, const std::string& delimiter, const std::string& end) {
  return {std::cout, start, delimiter, end};
}

template<typename Parent, typename Args>
struct Print {
  using InputType = typename Parent::OutputType;

  Parent parent;
  Args args;

  struct Execution : public ExecutionBase {
    Execution(const Args& args): args(args) {}

    Args args;
    auto_val(printed, false);
    auto_val(ctrl, default_control());

    inline auto& control() {
      return ctrl;
    }

    inline void start() {}

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

    template<typename Exec, typename ... ArgT>
    static void execute(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.start();
      exec.process();
      exec.end();
    }
  };

  inline decltype(auto) execute() {
    return parent.template wrap<ExecutionType::Execute, Execution, Args&>(args);
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, PrintArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args args) {
  return Print<P, A>{
    std::forward<Parent>(parent),
    std::forward<Args>(args)
  }.execute();
}
} // namespace coll
