#include <iostream>

#include "collection/coll.hpp"

int main() {
  int K = 5;

  auto topk_freq_words_of_diff_lens = 
    // read char stream from std::cin
    coll::iterate(std::istreambuf_iterator<char>(std::cin.rdbuf()), {})
      | coll::map(anony_cc(char(std::tolower(_))))
      // split the char stream into strings
      | coll::split(std::string(), anony_cc(!std::isalnum(_) && _ != '_'))
      // group string and count occurrences, have to process all inputs before we know the count of a string
      | coll::groupby(anony_ac(_)).count()
      | coll::iterate()
      | coll::map(anony_ac(std::make_pair(_.first, _.second))) // remove const in key
      // group the (string, count) by string.length() and take the top k strings with highest occurrences
      // have to process all inputs before we know the top n words under each length
      | coll::groupby(anony_ac(_.first.length()))
          .aggregate(coll::topk(K).by(anony_ac(_.second)),
            [](auto& topk, auto&& str_cnt) { topk.push(str_cnt); });

  // print the results nicely
  coll::iterate(topk_freq_words_of_diff_lens)
    | coll::println().format([](auto& out, auto& len2topk) {
        auto& topk = len2topk.second;
        // default element order in topk is from smallest to largest
        coll::generate([&]() { auto s = topk.top(); topk.pop(); return s; })
            .until(anonyr_c(topk.empty()))
          | coll::reverse().with_buffer()
          | coll::print(std::to_string(len2topk.first) + ": [", ", ", "]")
              .format([](auto& out, auto& str_cnt) {
                out << '(' << str_cnt.first << ", " << str_cnt.second << ')';
              });
      });
}
