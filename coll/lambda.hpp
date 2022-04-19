#pragma once
/**
 * Generated by examples/lambda_generator.cpp
 * anony stands for anonymous
 * r stands for reference
 * c stands for copy
 * a stands for any
 * v stands for void
 * m stands for mutable
 **/
#define anony_v(...)    ([ ](                        )                           { __VA_ARGS__; })
#define anony_c(...)    ([ ](                        )                           { return (__VA_ARGS__); })
#define anony_r(...)    ([ ](                        ) -> auto&                  { return (__VA_ARGS__); })
#define anony_a(...)    ([ ](                        ) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyr_v(...)   ([&](                        )                           { __VA_ARGS__; })
#define anonyr_c(...)   ([&](                        )                           { return (__VA_ARGS__); })
#define anonyr_r(...)   ([&](                        ) -> auto&                  { return (__VA_ARGS__); })
#define anonyr_a(...)   ([&](                        ) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_v(...)   ([=](                        )                           { __VA_ARGS__; })
#define anonyc_vm(...)  ([=](                        )                   mutable { __VA_ARGS__; })
#define anonyc_c(...)   ([=](                        )                           { return (__VA_ARGS__); })
#define anonyc_cm(...)  ([=](                        )                   mutable { return (__VA_ARGS__); })
#define anonyc_r(...)   ([=](                        ) -> auto&                  { return (__VA_ARGS__); })
#define anonyc_rm(...)  ([=](                        ) -> auto&          mutable { return (__VA_ARGS__); })
#define anonyc_a(...)   ([=](                        ) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_am(...)  ([=](                        ) -> decltype(auto) mutable { return (__VA_ARGS__); })
#define anony_cv(...)   ([ ]([[maybe_unused]]auto   _)                           { __VA_ARGS__; })
#define anony_cc(...)   ([ ]([[maybe_unused]]auto   _)                           { return (__VA_ARGS__); })
#define anony_cr(...)   ([ ]([[maybe_unused]]auto   _) -> auto&                  { return (__VA_ARGS__); })
#define anony_ca(...)   ([ ]([[maybe_unused]]auto   _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyr_cv(...)  ([&]([[maybe_unused]]auto   _)                           { __VA_ARGS__; })
#define anonyr_cc(...)  ([&]([[maybe_unused]]auto   _)                           { return (__VA_ARGS__); })
#define anonyr_cr(...)  ([&]([[maybe_unused]]auto   _) -> auto&                  { return (__VA_ARGS__); })
#define anonyr_ca(...)  ([&]([[maybe_unused]]auto   _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_cv(...)  ([=]([[maybe_unused]]auto   _)                           { __VA_ARGS__; })
#define anonyc_cvm(...) ([=]([[maybe_unused]]auto   _)                   mutable { __VA_ARGS__; })
#define anonyc_cc(...)  ([=]([[maybe_unused]]auto   _)                           { return (__VA_ARGS__); })
#define anonyc_ccm(...) ([=]([[maybe_unused]]auto   _)                   mutable { return (__VA_ARGS__); })
#define anonyc_cr(...)  ([=]([[maybe_unused]]auto   _) -> auto&                  { return (__VA_ARGS__); })
#define anonyc_crm(...) ([=]([[maybe_unused]]auto   _) -> auto&          mutable { return (__VA_ARGS__); })
#define anonyc_ca(...)  ([=]([[maybe_unused]]auto   _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_cam(...) ([=]([[maybe_unused]]auto   _) -> decltype(auto) mutable { return (__VA_ARGS__); })
#define anony_rv(...)   ([ ]([[maybe_unused]]auto&  _)                           { __VA_ARGS__; })
#define anony_rc(...)   ([ ]([[maybe_unused]]auto&  _)                           { return (__VA_ARGS__); })
#define anony_rr(...)   ([ ]([[maybe_unused]]auto&  _) -> auto&                  { return (__VA_ARGS__); })
#define anony_ra(...)   ([ ]([[maybe_unused]]auto&  _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyr_rv(...)  ([&]([[maybe_unused]]auto&  _)                           { __VA_ARGS__; })
#define anonyr_rc(...)  ([&]([[maybe_unused]]auto&  _)                           { return (__VA_ARGS__); })
#define anonyr_rr(...)  ([&]([[maybe_unused]]auto&  _) -> auto&                  { return (__VA_ARGS__); })
#define anonyr_ra(...)  ([&]([[maybe_unused]]auto&  _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_rv(...)  ([=]([[maybe_unused]]auto&  _)                           { __VA_ARGS__; })
#define anonyc_rvm(...) ([=]([[maybe_unused]]auto&  _)                   mutable { __VA_ARGS__; })
#define anonyc_rc(...)  ([=]([[maybe_unused]]auto&  _)                           { return (__VA_ARGS__); })
#define anonyc_rcm(...) ([=]([[maybe_unused]]auto&  _)                   mutable { return (__VA_ARGS__); })
#define anonyc_rr(...)  ([=]([[maybe_unused]]auto&  _) -> auto&                  { return (__VA_ARGS__); })
#define anonyc_rrm(...) ([=]([[maybe_unused]]auto&  _) -> auto&          mutable { return (__VA_ARGS__); })
#define anonyc_ra(...)  ([=]([[maybe_unused]]auto&  _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_ram(...) ([=]([[maybe_unused]]auto&  _) -> decltype(auto) mutable { return (__VA_ARGS__); })
#define anony_av(...)   ([ ]([[maybe_unused]]auto&& _)                           { __VA_ARGS__; })
#define anony_ac(...)   ([ ]([[maybe_unused]]auto&& _)                           { return (__VA_ARGS__); })
#define anony_ar(...)   ([ ]([[maybe_unused]]auto&& _) -> auto&                  { return (__VA_ARGS__); })
#define anony_aa(...)   ([ ]([[maybe_unused]]auto&& _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyr_av(...)  ([&]([[maybe_unused]]auto&& _)                           { __VA_ARGS__; })
#define anonyr_ac(...)  ([&]([[maybe_unused]]auto&& _)                           { return (__VA_ARGS__); })
#define anonyr_ar(...)  ([&]([[maybe_unused]]auto&& _) -> auto&                  { return (__VA_ARGS__); })
#define anonyr_aa(...)  ([&]([[maybe_unused]]auto&& _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_av(...)  ([=]([[maybe_unused]]auto&& _)                           { __VA_ARGS__; })
#define anonyc_avm(...) ([=]([[maybe_unused]]auto&& _)                   mutable { __VA_ARGS__; })
#define anonyc_ac(...)  ([=]([[maybe_unused]]auto&& _)                           { return (__VA_ARGS__); })
#define anonyc_acm(...) ([=]([[maybe_unused]]auto&& _)                   mutable { return (__VA_ARGS__); })
#define anonyc_ar(...)  ([=]([[maybe_unused]]auto&& _) -> auto&                  { return (__VA_ARGS__); })
#define anonyc_arm(...) ([=]([[maybe_unused]]auto&& _) -> auto&          mutable { return (__VA_ARGS__); })
#define anonyc_aa(...)  ([=]([[maybe_unused]]auto&& _) -> decltype(auto)         { return (__VA_ARGS__); })
#define anonyc_aam(...) ([=]([[maybe_unused]]auto&& _) -> decltype(auto) mutable { return (__VA_ARGS__); })
