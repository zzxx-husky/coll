#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "collection/coll.hpp"

template <template<typename ...> class C>
void run(const std::string& str) {
  auto c = coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::to<C>();
  std::cout << "Results using to<" << str << ">()."
    << " Input elems: " << str.size() << ". Output elems: " << c.size() << '.' << std::endl;
}

template <template<typename ...> class C>
void run_with_container(const std::string& str) {
  C<char> c;
  coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::to(c);
  std::cout << "Results using to<" << str << ">()."
    << " Input elems: " << str.size() << ". Output elems: " << c.size() << '.' << std::endl;
}

template <template<typename ...> class C>
void run_with_reservation(const std::string& str) {
  auto c = coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::to([sz = str.size()](auto type) {
        C<typename decltype(type)::type> c;
        c.reserve(sz);
        return c;
      });
  std::cout << "Results using to<" << str << ">()."
    << " Input elems: " << str.size() << ". Output elems: " << c.size() << '.' << std::endl;
}

template <template<typename ...> class C>
void run_with_map(const std::string& str) {
  auto c = coll::iterate(str)
    | coll::map(anony_cc(char(std::toupper(_))))
    | coll::map(anony_cc(std::make_pair(_, _)))
    | coll::to(C<char, char>());
  std::cout << "Results using to<" << str << ">()."
    << " Input elems: " << str.size() << ". Output elems: " << c.size() << '.' << std::endl;
}

int main() {
  run<std::deque>("deque");
  run<std::list>("list");
  run<std::priority_queue>("priority_queue");
  run<std::queue>("queue");
  run<std::set>("set");
  run<std::stack>("stack");
  run<std::unordered_set>("unordered_set");
  run<std::vector>("vector");

  run_with_container<std::deque>("deque");
  run_with_container<std::list>("list");
  run_with_container<std::priority_queue>("priority_queue");
  run_with_container<std::queue>("queue");
  run_with_container<std::set>("set");
  run_with_container<std::stack>("stack");
  run_with_container<std::unordered_set>("unordered_set");
  run_with_container<std::vector>("vector");

  run_with_reservation<std::unordered_set>("unordered_set");
  run_with_reservation<std::vector>("vector");

  run_with_map<std::map>("map");
  run_with_map<std::unordered_map>("unordered_map");
}
