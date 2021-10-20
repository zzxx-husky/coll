# Coll

[![CMake](https://github.com/zzxx-husky/coll/actions/workflows/cmake.yml/badge.svg)](https://github.com/zzxx-husky/coll/actions/workflows/cmake.yml)

A set of APIs for constructing a pipeline for data processing based on c++17.
Data processing is expressed as a pipeline of `operator`s connected by `|`. 

The implementation of `coll` is not iterater-based nor generator-based. Instead, it is based on function nesting (`函数套娃`)
  and relies on the optimizations of the compiler to reduce the overhead of the API,
  such that the processing logic would be close to using native c++ syntax (e.g., `for`, `while`, `if-else`) to do the same stuffs.

The optimizations are typically
  function inlining,
  return value optimization (RVO),
  named return value optimization (NRVO).

`coll` is not designed for functional programming (FP is typically composed of pure functions and avoids shared state, mutable data and side-effects).

`coll` also supports `parallel` operator for parallel pipeline processing with the help of [ZAF](https://github.com/zzxx-husky/zaf).

# Explain Coll in Short

1. A sink operator triggers the construction of an execution object that wraps the logics of all the operators in the pipeline (for lazy evaluation).
2. The logic of the child operator is typically *nested* inside the logic of the parent operator.
3. The execution starts from the source operator(s), where the data are actively pushed by the parent operator to the child operator.
4. With function inlining, we expect the logics of the *coll operators* (i.e., *lambda calls*) are well inlined by the compiler.

```c++
/* Count the occurrence of character in HelloWorld */                                                                                 auto counts = []() {
auto counts = iterate("HelloWorld")      iterate.proc = () {         +-- map.proc = (i) {          +-- groupby.count.proc = (i) {       unordered_map<char, size_t> counts_map;
  | map(anony_cc(char(to_lower(_))))       for (i : "HelloWorld") {  |     r = char(to_lower(i));  |     counts_map[i]++;               for (auto i : "HelloWorld") {
  | groupby().count()                 =      map(i)  <---------------+     groupby.count(r);  <----+   }                           =      auto c = char(to_lower(i));
                                           }                             }                                                                counts_map[c]++;
                                         }                                                                                              }
                                                                                                                                        return counts_map;
                                                                                                                                      }();

/* Generate 10 random numbers     */          generate.times.proc = () {  +-- branch.proc = (i) {      +-- sort.proc = (i) {               []() {
/* 1. print them in sorted order  */            for (i = 0; i++ < 10;) {  |     /*1st branch*/         |     sort_buf.emplace_back(i)        vector<int> sort_buf;
/* 2. remove duplicates and print */              auto x = rand() % 100   |     sort(i)      <---------+   }                                 unordered_set<int> dist_buf;
generate(anony_c(rand()%100)).times(10)           branch(x)  <------------+     /*2nd branch*/                                               for (auto i = 0; i < 10; i++) {
  | branch([](auto in) {                        }                               distinct(i)  <------------ distinct.proc = (i) {               auto x = rand() % 100;
      in | sort()                             }                               }                              if (dist_buf.emplace(i))          sort_buf.emplace_back(x);
         | println()                    =                                                              +-->    println(i)               =      if (dist_buf.emplace(i).second) {
    })                                        generate.times.end = () {   +-- sort.end = () {          |     }                                   cout << i << "\n";
  | distinct()                            +-->  branch.end()              |     sort(sort_buf)         |   }                                   }
  | println()                             |   }                           |     for (i : sort_buf) {   |                                     }
                                          |                               |       println(i)  <--------+-- println.proc = (i) {              sort(sort_buf.begin(), sort_buf.end());
                                          +-- branch.end = () {           |     }                            cout << i;                      for (auto i : sort_buf) {
                                                sort.end()     <----------+     println.end() <--------+   }                                   cout << i << "\n";
                                                distinct.end()                }                        |                                     }
                                              }                                                        +-- println.end = (){}              }()
                                                                              distinct.end = () {      |
                                                                                println.end() <--------+
                                                                              }
```

# The HelloWorld Example

Here is the HelloWorld example which can be found in `examples/hello_world.cpp` and `examples/hello_world_native.cpp`.
The pipeline on the left side using `coll` is expected to be close (after optimized by the compiler) to the processing on the right side using native c++ syntax.

What this example does is simple: we select "hello" and "world" from the input elements, then transform each character to upper case and print them out.
The example uses a lots of operator for the purpose of showing how different operators are translated into native c++ syntax.

```c++
 1| elements("let us", "say", "hello")            1| bool printed_head = false;
 2|   | tail()                                    2| size_t index = 0;
 3|   | concat(                                   3| array<const char*, 3> in1{"let us", "say", "hello"};
 4|       elements("to the", "world", "loudly")   4| bool skipped_head = false;
 5|         | init()                              5| for (auto& i1 : in1) { // elements()
 6|     )                                         6|   if (skipped_head) { // tail()
 7|   | zip_with_index()                          7|     pair<size_t, const char*&> idx_i1{index++, i1}; // zip_with_index()
 8|   | filter(anony_ac(_.first % 2 == 1)         8|     if (idx_i1.first % 2 == 1) { // filter()
 9|   | map(anony_ar(_.second))                   9|       auto& i1_ref = idx_i1.second; // map()
10|   | flatten()                                10|       for (auto c = i1_ref; *c; c++) { // flatten()
11|   | map(anony_ac(char(toupper(_))))          11|         auto C = char(toupper(*c)); // map()
12|   | print("", " ", "!\n");                   12|         if (printed_head) { // print()
                                                 13|           cout << " ";
                                                 14|         } else {
                                                 15|           cout << "";
                                                 16|           printed_head = true;
                                                 17|         }
                                                 18|         cout << C;
                                                 19|       }
                                                 20|     }
                                                 21|   } else {
                                                 22|     skipped_head = true;
                                                 23|   }
                                                 24| }
                                                 25| array<const char*, 3> in2{"to the", "world", "loudly"};
                                                 26| optional<const char*> prev_i2;
                                                 27| for (auto& i2 : in2) { // elements()
                                                 28|   if (bool(prev_i2)) { // init()
                                                   |     ... // the same logic as from line 7 to line 20, concat()
                                                 43|   }
                                                 44|   prev_i2 = i2;
                                                 45| }
                                                 46| if (!printed_head) { // print()
                                                 47|   cout << "";
                                                 48| }
                                                 49| cout << "!\n";
```

Note that, the above code using native syntax can actually be improved:
1. When taking the `init` of `in1`, we can directly iterate from the `1st` element (zero based) to the `end`. No need to use `skipped_head` to check each element of `in1`;
2. When taking the `tail` of `in2`, we can directly iterate from the `begin` to the second last element. No need to use `prev_i2` to cache the previous element;

Now, worth to mention that, the two points can be optimized in `coll` by pushing `init` and `tail` upwards to the source operator in compile time.
That is not done yet, but similar technique is applied for `reverse` operator, i.e., instead of reversing the elements in the middle of the processing, it is better to directly reverse the input from the beginning if possible.

## TODOs:
+ API: window with aggregation;
+ Optimization: Improve parallel operators:
   + Direct data exchange among consecutive parallel operators

## Useful links
+ [Operator List](https://github.com/zzxx-husky/cpp-collection-api/wiki/OperatorList), which lists all the operators in `coll`.
+ [Range-v3](https://github.com/ericniebler/range-v3), which is the Range library for C++14/17/20, basis for C++20's std::ranges.
+ [rangeless](https://github.com/ast-al/rangeless), which has a good summary on many existing libraries that provide similar functionalies.
