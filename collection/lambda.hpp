#pragma once

#define anony_v(x)    ([ ](        ) -> void          {        (x); })
#define anonyr_v(x)   ([&](        ) -> void          {        (x); })
#define anonyc_v(x)   ([=](        ) -> void          {        (x); })
#define anonyc_vm(x)  ([=](        ) -> void  mutable {        (x); })

#define anony_c(x)    ([ ](        )                  { return (x); })
#define anonyr_c(x)   ([&](        )                  { return (x); })
#define anonyc_c(x)   ([=](        )                  { return (x); })
#define anonyc_cm(x)  ([=](        )          mutable { return (x); })

#define anony_cc(x)   ([ ](auto   _)                  { return (x); })
#define anonyr_cr(x)  ([&](auto   _) -> auto&         { return (x); })
#define anonyr_cc(x)  ([&](auto   _)                  { return (x); })
#define anonyc_cr(x)  ([=](auto   _) -> auto&         { return (x); })
#define anonyc_cc(x)  ([=](auto   _)                  { return (x); })
#define anonyc_crm(x) ([=](auto   _) -> auto& mutable { return (x); })
#define anonyc_ccm(x) ([=](auto   _)          mutable { return (x); })

#define anony_rr(x)   ([ ](auto&  _) -> auto&         { return (x); })
#define anony_rc(x)   ([ ](auto&  _)                  { return (x); })
#define anonyr_rr(x)  ([&](auto&  _) -> auto&         { return (x); })
#define anonyr_rc(x)  ([&](auto&  _)                  { return (x); })
#define anonyc_rr(x)  ([=](auto&  _) -> auto&         { return (x); })
#define anonyc_rc(x)  ([=](auto&  _)                  { return (x); })
#define anonyc_rrm(x) ([=](auto&  _) -> auto& mutable { return (x); })
#define anonyc_rcm(x) ([=](auto&  _)          mutable { return (x); })

#define anony_ar(x)   ([ ](auto&& _) -> auto&         { return (x); })
#define anony_ac(x)   ([ ](auto&& _)                  { return (x); })
#define anonyr_ar(x)  ([&](auto&& _) -> auto&         { return (x); })
#define anonyr_ac(x)  ([&](auto&& _)                  { return (x); })
#define anonyc_ar(x)  ([=](auto&& _) -> auto&         { return (x); })
#define anonyc_ac(x)  ([=](auto&& _)                  { return (x); })
#define anonyc_arm(x) ([=](auto&& _) -> auto& mutable { return (x); })
#define anonyc_acm(x) ([=](auto&& _)          mutable { return (x); })
