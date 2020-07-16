#include "collection/coll.hpp"

#include "scapegoat.hpp"

int main() {
  auto vals = coll::range(20)
    | coll::map(anony_cc(Scapegoat{rand() % 20}))
    | coll::to<std::vector>();

  coll::iterate(vals)
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");

  std::cout << "Init" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::init()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Init + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::init().cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Init + reverse" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::init()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Init + reverse + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::init().cache_by_ref()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Tail" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::tail()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Tail + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::tail().cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Tail + reverse" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::tail()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "Tail + reverse + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::iterate(vals) 
    | coll::tail().cache_by_ref()
    | coll::reverse()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "IterateByNext + Tail + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::generate([&, i = 0]() mutable -> auto& { return vals[i++]; })
      .until([&, i = 0]() mutable { return i++ == vals.size(); })
    | coll::tail().cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "IterateByNext + Tail + reverse + buffer" << std::endl;
  ScapegoatCounter::clear();
  coll::generate([&, i = 0]() mutable -> auto& { return vals[i++]; })
      .until([&, i = 0]() mutable { return i++ == vals.size(); })
    | coll::tail().cache_by_ref()
    | coll::reverse().with_buffer()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();

  std::cout << "IterateByNext + Tail + reverse + buffer + ref" << std::endl;
  ScapegoatCounter::clear();
  coll::generate([&, i = 0]() mutable -> auto& { return vals[i++]; })
      .until([&, i = 0]() mutable { return i++ == vals.size(); })
    | coll::tail().cache_by_ref()
    | coll::reverse().with_buffer().cache_by_ref()
    | coll::map(anony_rr(_.val))
    | coll::print("[", ", ", "]\n");
  ScapegoatCounter::status();
}
