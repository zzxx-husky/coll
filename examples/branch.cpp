#include "collection/coll.hpp"

int main() {
  coll::generate(anony_c(rand() % 100)).times(10)
    | coll::branch([](auto in) {
        return in
           | coll::sort()
           | coll::map(anony_ac(std::make_pair("branch", _)))
           | coll::print("\n", "\n", "\n")
               .format([](auto& o, auto&& i) {
                 o << '(' << i.first << ", " << i.second << ')';
               });
      })
    | coll::distinct()
    | coll::map(anony_ac(std::make_pair("output", _)))
    | coll::print("\n", "\n", "\n")
        .format([](auto& o, auto&& i) {
          o << '(' << i.first << ", " << i.second << ')';
        });
}
