#include "collection/coll.hpp"

int main() {
  coll::elements("let us", "say", "hello")
    // ["let us", "say", "hello"]
    | coll::tail()
    // ["say", "hello"]
    | coll::concat(
        coll::elements("to the", "world", "loudly")
          // ["to the", "world", "loudly"]
          | coll::init()
          // ["to the", "world"]
      )
    // ["say", "hello", "to the", "world"]
    | coll::zip_with_index()
    // [(0, "say"), (1, "hello"), (2, "to the"), (3, "world")]
    | coll::filter(anony_ac(_.first % 2 == 1))
    // [(1, "hello"), (3, "world")]
    | coll::map(anony_ar(_.second))
    // ["hello", "world"]
    | coll::flatten()
    // ['h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd']
    | coll::map(anony_ac(char(std::toupper(_))))
    // ['H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D']
    | coll::print("", " ", "!\n");
    // std::cout << ""'H'" "'E'" "'L'" "'L'" "'O'" "'W'" "'O'" "'R'" "'L'" "'D'"!\n"
}
