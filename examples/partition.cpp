#include "collection/coll.hpp"

int main() {
  coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in
          | coll::map(anonyc_cc(std::make_pair(key, _)))
          | coll::window(5)
          | coll::print("", "", "").format([](auto& out, auto& w) {
              coll::iterate(w)
                | coll::print().format([](auto& out, auto& pair) {
                    out << '(' << pair.first << ", " << pair.second << ')';
                  });
            });
      })
      .by(anony_cc(_ % 4))
    | coll::inspect([](auto k) {
        std::cout << "Key " << k << " ends." << std::endl;
      })
    | coll::act();

  coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in | coll::sum();
      })
      .by(anony_cc(_ % 4))
    | coll::inspect([](auto&& k2s) {
        std::cout << "Key " << k2s.first << " ends with sum " << *k2s.second << std::endl;
      })
    | coll::act();

  coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in
          | coll::zip_with_index()
          | coll::map(anonyc_cc(std::make_pair(key, _)));
      })
      .by(anony_cc(_ % 4))
    | coll::print().format([](auto& out, auto&& e) {
        out << "(" << e.first << ", " << e.second.first << ", " << e.second.second << ")";
      });

  coll::range(100)
    | coll::partition([](auto& key, auto in) {
        return in | coll::sort();
      })
      .by(anony_cc(_ % 5))
    | coll::print();
}
