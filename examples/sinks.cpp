#include <algorithm>
#include <numeric>
#include <vector>

#include "collection/coll.hpp"

#include "scapegoat.hpp"

int main() {
  auto v = coll::range(20)
    | coll::map(anony_cc(Scapegoat{rand() % 1000}))
    | coll::to<std::vector>();

  std::cout << "vector::size: " << v.size() << std::endl;

  std::cout << "coll::count: "
    << (coll::iterate(v) | coll::count()) << std::endl;

  std::cout << "std::max_element: "
    << *std::max_element(v.begin(), v.end()) << std::endl;

  ScapegoatCounter::clear();
  std::cout << "coll::max: "
    << (coll::iterate(v) | coll::max()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  ScapegoatCounter::clear();
  std::cout << "coll::max.ref: "
    << (coll::iterate(v) | coll::max().ref()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  std::cout << "std::min_element: "
    << *std::min_element(v.begin(), v.end()) << std::endl;

  ScapegoatCounter::clear();
  std::cout << "coll::min: "
    << (coll::iterate(v) | coll::min()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  ScapegoatCounter::clear();
  std::cout << "coll::min.ref: "
    << (coll::iterate(v) | coll::min().ref()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  std::cout << "std::accumulate sum: "
    << std::accumulate(v.begin(), v.end(), Scapegoat{0}) << std::endl;

  std::cout << "coll::sum: "
    << (coll::iterate(v) | coll::sum()).value_or(-1) << std::endl;

  std::cout << "std::accumulate avg: "
    << std::accumulate(v.begin(), v.end(), Scapegoat{0}).val / v.size() << std::endl;

  std::cout << "coll::avg: "
    << (coll::iterate(v)
          | coll::map(anony_rr(_.val))
          | coll::avg()
       ).value_or(-1)
    << std::endl;

  std::cout << "std::accumulate avg (double): "
    << std::accumulate(v.begin(), v.end(), 0.) / v.size() << std::endl;

  std::cout << "coll::avg (double): "
    << (coll::iterate(v)
          | coll::map(anony_rr(_.val))
          | coll::avg().init(0.)
       ).value_or(-1)
    << std::endl;
  
  std::cout << "vector::front: " << v.front() << std::endl;

  ScapegoatCounter::clear();
  std::cout << "coll::head: "
    << (coll::iterate(v) | coll::head()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  ScapegoatCounter::clear();
  std::cout << "coll::head.ref: "
    << (coll::iterate(v) | coll::head().ref()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  std::cout << "vector::back: " << v.back() << std::endl;

  ScapegoatCounter::clear();
  std::cout << "coll::last: "
    << (coll::iterate(v) | coll::last()).value_or(-1) << std::endl;
  ScapegoatCounter::status();

  ScapegoatCounter::clear();
  std::cout << "coll::last.ref: "
    << (coll::iterate(v) | coll::last().ref()).value_or(-1) << std::endl;
  ScapegoatCounter::status();
}
