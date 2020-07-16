#pragma once

#include "base.hpp"

namespace coll {
template<typename CollCtor>
struct BranchArgs {
  CollCtor ctor;
};

template<typename Parent, typename OutputCtrl, typename OutputProc>
struct BranchHelper {
  using OutputType = typename Parent::OutputType;

  Parent& parent;
  OutputCtrl& output_ctrl;
  OutputProc& output_proc;

  template<typename BranchCtrl, typename BranchProc>
  inline void foreach(BranchCtrl& branch_ctrl, BranchProc branch_proc) {
    parent.foreach(output_ctrl, output_proc, branch_ctrl, branch_proc);
  }
};

template<typename Parent, typename Args>
struct Branch {
  using InputType = typename Parent::OutputType;
  using OutputType = InputType;

  Parent parent;
  Args args;

  template<typename OutputCtrl, typename OutputProc, typename BranchCtrl, typename BranchProc>
  void foreach(OutputCtrl& output_ctrl, OutputProc& output_proc /* kept in Branch::foreach */,
               BranchCtrl& branch_ctrl, BranchProc& branch_proc /* kept in BranchHelper::foreach */) {
    // check the consistency of the two ctrls, e.g., reverse flag
    static_assert(OutputCtrl::is_reversed == BranchCtrl::is_reversed,
      "The output requires a different input order from the branch"); 
    auto new_ctrl = default_control();
    parent.foreach(new_ctrl,
      [&](InputType e) {
        if (unlikely(!branch_ctrl.break_now)) { 
          branch_proc(e);
        }
        if (unlikely(!output_ctrl.break_now)) {
          output_proc(std::forward<InputType>(e));
        }
        if (unlikely(branch_ctrl.break_now && output_ctrl.break_now)) {
          new_ctrl.break_now = true;
        }
      });
  }

  template<typename Ctrl, typename ChildProc>
  void foreach(Ctrl& output_ctrl, ChildProc output_proc) {
    args.ctor(BranchHelper<Branch<Parent, Args>, Ctrl, ChildProc> {
      *this, output_ctrl, output_proc
    });
  }
};

template<typename CollCtor>
inline BranchArgs<CollCtor> branch(CollCtor ctor) {
  return {std::forward<CollCtor>(ctor)};
}

template<typename Parent, typename CollCtor,
  std::enable_if_t<traits::is_collection<Parent>::value>* = nullptr>
inline Branch<Parent, BranchArgs<CollCtor>>
operator | (const Parent& parent, BranchArgs<CollCtor> args) {
  return {parent, std::move(args)};
}
} // namespace coll
