#include <iostream>

#include "coll/coll.hpp"

int main() {
  srand(time(0));

  auto n = 1000000000000000ll;
  // generate n random integers and take the max of the first 3 integers
  auto res = coll::generate(anony_c(rand())).times(n)
    | coll::take_first(3)
    | coll::inspect([i = 0](auto&& x) mutable {
        // let us see which 3 integers
        std::cout << "Int " << (++i) << " is " << x << std::endl;
      })
    | coll::max();

  std::cout << "The max of the 3 ints is " << *res << std::endl;
}
