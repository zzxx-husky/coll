#include "collection/coll.hpp"

#include "scapegoat.hpp"

int main() {
  auto vals = coll::range(20)
    | coll::map(anony_cc(Scapegoat{rand() % 20}))
    | coll::to<std::vector>();

  std::cout << "window(1)" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(1)
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5)" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5)
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5, 1)" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5, 1)
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5, 1) + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5, 1).cache_by_ref()
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5, 4) + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5, 4).cache_by_ref()
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5, 4) + ref + reverse" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5, 4).cache_by_ref()
    | coll::reverse().with_buffer()
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5, 4) + val + reverse" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5, 4)
    | coll::reverse().with_buffer()
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();

  std::cout << "window(5, 6) + ref + reverse" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals)
    | coll::window(5, 6).cache_by_ref()
    | coll::reverse().with_buffer()
    | coll::print().format([](auto& out, auto& w) { 
        coll::iterate(w)
          | coll::map(anony_rr(_.val))
          | coll::print("[", ", ", "]").to(out);
      });
  ScapegoatCounter::status();


}
