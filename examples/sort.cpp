#include <queue>

#include "collection/coll.hpp"

#include "scapegoat.hpp"

int main() {
  auto vals = coll::range(20)
    | coll::map(anony_cc(Scapegoat{rand() % 20}))
    | coll::to<std::vector>();

  std::cout << "Forward Sort by val." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Forward Sort by val with unique." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
    | coll::unique()
      .by(anony_rc(_.val))
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Forward Sort by ref with unique by ref." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort().cache_by_ref()
    | coll::unique()
      .by(anony_rr(_))
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Reverse Sort by val." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
      .reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Reverse Sort by val." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
      .by([](auto& a, auto& b) { return b < a; })
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Forward Sort by val." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
      .by(anony_rc(_.val))
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Forward Sort by val." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
      .by([](auto& a, auto& b) { return b < a; })
      .reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::priority_queue<int> queue;

  std::cout << "Reverse Sort by ref." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::inspect([&](auto& e) {
        queue.push(e.val);
      })
    | coll::sort()
      .by([](auto& a, auto& b) { return b < a; })
      .cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Reverse Sort by ref then Reverse." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort()
      .by([](auto& a, auto& b) { return b < a; })
      .cache_by_ref()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Sort by ref then to vector." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort().cache_by_ref()
    // The output of sort is Scapegoat, but the buffer type is Reference<Scapegoat>
    // We can only move a buffer storing Reference if we want to avoid copy
    | coll::to<std::vector<coll::Reference<Scapegoat>>>()
    | coll::iterate()
    | coll::map(anony_rr(_->val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Sort by ref then map then to vector." << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::sort().cache_by_ref()
    | coll::map(anony_rr(_))
    | coll::to<std::vector>()
    | coll::iterate()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Priority Queue." << std::endl;
  coll::generate([&]() {
        int max_val = queue.top();
        queue.pop();
        return max_val;
      }).until(anonyr_c(queue.empty())) 
    | coll::print("[", ", ", "]\n");
}
