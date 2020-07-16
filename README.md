# CPP Collection

A small tookit (called `coll` for short) for constructing a pipeline for data processing in c++17.

User expresses the processing logic as a pipeline.
  The API of `coll` is similar to the Range-v3 library and using `operaotr |` for pipelining.

The implementation of `coll` is not iterater-based nor generator-based. Instead, it is based on function nesting (`函数套娃`)
  and relies on the optimizations of the compiler to reduce the overhead of the API,
  such that the processing logic would be close to using native c++ syntax (e.g., `for`, `while`, `if-else`) to do the same stuffs.

The optimizations are typically
  function inlining,
  return value optimization (RVO),
  named return value optimization (NRVO).

`coll` is not designed for functional programming (i.e., FP).
  FP is typically composed of pure functions and avoids shared state, mutable data and side-effects.

Downside: The template of an operaor carries the information of all the parent operators, which makes it challenging to fix compilation errors.

Useful links:
+ [Range-v3](https://github.com/ericniebler/range-v3), which is the Range library for C++14/17/20, basis for C++20's std::ranges
+ [rangeless](https://github.com/ast-al/rangeless), which has a good summary on many existing libraries that provide similar functionalies.

# Explain Function Nesting by Short Examples

1. A sink operator triggers the construction of a unified lambda that wraps the logics of all the operators in the pipeline. Thus, this is lazy evaluation.
2. The logic of the child operator is typically *nested* inside the logic of the parent operator.
3. The execution starts from the source operator(s), where the data are actively pushed by the parent operator to the child operator.
4. With function inlining, we expect that *coll operator* and *lambda call* would be optimized out by the compiler.
5. For a `branch` operator that connects two child operators, each input is forwarded to one child and then to the other child.
6. For a `concat` operator that connects two parent operators, drain the outpus of one parent and then drain the other parent.

```c++
/* Count the occurrence of character in HelloWorld */                                                                                 auto counts = []() {
auto counts = iterate("HelloWorld")      iterate = () {              +-- map = (auto i) {          +-- groupby.count = (auto i) {       unordered_map<char, size_t> counts_map;
  | map(anony_cc(char(to_lower(_))))       for (i : "HelloWorld") {  |     r = char(to_lower(i));  |     counts_map[i]++;               for (auto i : "HelloWorld") {
  | groupby().count()                 =      map(i)  <---------------+     groupby.count(r);  <----+   }                           =      auto c = char(to_lower(i));
                                           }                             }                                                                counts_map[c]++;
                                         }                                                                                              }
                                                                                                                                        return counts_map;
                                                                                                                                      }();

/* Generate 10 random numbers                     */                                                                                  []() {
/* On the one hand, print them in sorted order    */                                                                                    vector<int> sort_buf;
/* on the other hand, remove duplicates and print */                                                                                    unordered_set<int> dist_buf;
generate(anony_c(rand()%100)).times(10)  generate.times = () {       +-- branch = (auto i) {       +-- sort = (auto i) {                for (auto i = 0; i < 10; i++) {
  | branch([](auto in) {                   for (i = 0; i++ < 10;) {  |     /*1st branch*/          |     sort_buf.emplace_back(i)         auto x = rand() % 100;
      in | sort                              auto x = rand() % 100   |     sort(i)      <----------+   }                                  sort_buf.emplace_back(x);
         | println()                  =      branch(x)  <------------+     /*2nd branch*/                                          =      if (dist_buf.emplace(i).second) {
    }                                      }                               distinct(i)  <------------- distinct = (auto i) {                cout << i << "\n";
  | distinct                             }                               }                               if (dist_buf.emplace(i)) {       }
  | println()                                                                                              println(i)                   }
                                                                                                         }                              sort(sort_buf.begin(), sort_buf.end());
                                                                                                       }                                for (auto i : sort_buf) {
                                                                                                                                          cout << i << "\n";
                                                                                                                                        }
                                                                                                                                      }()
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
1. When taking the `init` of `in1`, we can directly iterate from the `1th` element (zero based) to the `end`. No need to use `skipped_head` to check each element of `in1`;
2. When taking the `tail` of `in2`, we can directly iterate from the `begin` to the second last element. No need to use `prev_i2` to cache the previous element;
3. When checking whether the `index` of the elements is `_ % 2 == 1`, it is not necessary to zip the element with the index using a pair.

Now, worth to mention that, for point 1 and point 2, they can be optimized in `coll` by pushing `init` and `tail` upwards to the source operator in compile time.
That is not done yet, but similar technique is applied for `reverse` operator, i.e., instead of reversing the elements in the middle of the processing, it is better to directly reverse the input from the beginning if possible.
For point 3, it is typically what we hope the compiler will optimize for us, i.e., the overhead of using a `pair` can be removed by the compiler.

## TODOs:
1. Optimize: window with step=1 can actually support reversion;
1. Optimize: window can actually support reversion if the num of inputs is known in advance;
2. API: window with aggregation;
3. API: window aggregation with groupby;
4. API: native support for parallel processing;

## Operator List
| Operator | Input &#8594; Output | Description |
| :---- | :---- | :---- |
| _Source Operator_ | | |
| iterate | iterable &#8658; O | Iterate elements in a temporal or persistent iterable or a range. <br> **(Iterable i)**: iterate elements in iterable `i`. <br> **(Iterator begin, Iterator end)**: iterate elements in a range specified by `[start, end)`. |
| range | [L, R) &#8658; O | Generate elements within a numeric range. <br> **(Num l, Num r)**: range (l &le; i &lt; r). <br> **(Num r)**: range (0 &le; i &lt; r). |
| generate | next( ) &#8658; O | Generate elements by user defined functions. <br> **(( )&#8594;O next)**: each invocation of `next` generates one element. <br> **.until(( )&#8594;bool condition)**: generation stops when `condition` outputs `true`; invoked before `next`. <br> **.times(int64)**: the num of elements to generate. |
| elements | (O...) &#8658; O | Iterate each of the given elements to output <br> **(O... e...)**: elements to iterate.|
| _Pipeline Operator_ | | |
| map | I &#8594; O | Transform one input to one output, e.g., a new value, a member of the input, or a reference to an external variable. <br> **(I&#8594;O mapper)**: map each input to an output. |
| zip_with_index | I &#8594; (int64, I) | Output a pair with `second` to be the input and `first` to be the index of the input. <br> The index starts from 0 and increases by 1 for each input. |
| window | I &#8594; [I] | Output inputs in windows. Each input appears in N &ge; 0 window(s). The last window may be incomplete but not empty. <br> **(int64 s)**: creates tumbling window of size `s`. <br> **(int64 s, int64 t)**: create sliding windows of size `s` for every `t` inputs. <br> **.cache_by_ref( )**: use `Reference` to cache inputs to avoid copy. |
| foreach | I &#8594; I | Process each input with a UDF and forward the input to output. User can modify the input if it is a reference. <br> **(I&#8594;( ) proc)**: actions to take for each input. |
| filter | I &#8594; I | Forward to output the inputs that satisfy a given condition. <br> **(I&#8594;bool condition)**: check if an input satisfies the `condition`. |
| flatmap | I &#8594; [O] | Transform one input to N &ge; 0 output(s). <br> **(I&#8594;[O] mapper)**: map each input into an iterable or a `coll` operator. |
| flatten | [I] &#8594; I | Output the elements of the iterable input. |
| concat | I + I &#8594; I or <br> I + J &#8594; common<I, J> | Forward the output of the first parent and then forward the output of the second parent. |
| init <br> tail | I &#8594; I | Forward each input to output except the last / first input. <br> **.cache_by_ref()**: use Reference to cache inputs to avoid copy. |
| sort | I &#8594; I | Forward the inputs in a sorted order to output. A buffer is used to store all the inputs for sorting. <br> **.by((I, I)&#8594;bool cmp)**: specify a comparator to tell which input is *smaller*. <br> **.by(I&#8594;M mapper)**: map each input to a new value for comparison. <br> **.cache_by_ref()**: use Reference to cache inputs to avoid copy. <br> **.reverse()**: sort the inputs in a reverse order. |
| unique | I &#8594; I | Forwad an input to output if it does not duplicate with the previous input. <br> **.by(I &#8594; V mapper)**: map an input to a value or a reference for checking uniqueness. |
| reverse | I &#8594; I | Forward the inputs in a reverse order to output. Default strategy is to push reversion upwards until certain operators (e.g., source operator, sort) that can iterate elements reversely. <br> **.with_buffer( )**: use when reversion pushup fails due to non-reversable operators (e.g., window, generate). <br> **.cache_by_ref( )**: use `Reference` to cache inputs to avoid copy; available only if `.with_buffer()` is used. |
| distinct | I &#8594; I | Forward each input to output if it does not duplicate with any previous input. A set (default: unordered_set) is used to cache distinct inputs. <br> **.cache_by_ref( )**: use `Reference` to cache inputs to avoid copy. |
| split | I&#8594; vector\<I\> or <br> I &#8594; Container | Split the input stream by those inputs that satisfy a given condition and group adjacent inputs. <br> **(I&#8594;bool condition)**: the `condition` fot split. <br> **(Container s, I&#8594;bool condition)**: the `condition` for split with adjacent inputs grouped into `s`.
| branch | I &#8594; I | Make a new branch to process inputs in a different pipeline. <br> Each input is first forwarded to the branch and then to the output. <br> If no sink operator is used in the branch, the entire pipeline will not process any input. |
| _Sink Operator_ | | |
| groupby | I &#8594; (K, A) or <br> I &#8658; {K&#8594;A} | **(I&#8594;K key)**: extract a key from the input (default: identity function). <br> **.adjacent( )**: if used, only adjacent inputs with the same key are grouped and the processing remains pipelining; if not, a map storing the aggregation results for each key is produced. <br> **.valueby(I&#8594;V val)**: extract a value from the input for aggregation (default: identity function). <br> **.aggregate(A aggregator, (A&, V)&#8594;( ) aggregate)**, **.aggregate(Type\<I\>&#8594;A builder, (A&, V)&#8594;( ) aggregate)**: `aggregate` the values into `aggregator`. (default: insertion to a `vector`). <br> **.count( )**: count the number of inputs for each group. |
| act | I &#8594; ( ) | To trigger the processing of the pipeline. Used when the pipeline does not produce a result or write to an output stream. |
| aggregate | I &#8594; A | Aggregate inputs into an aggregator. <br> **(A aggregator, (A&, V)&#8594;( ) aggregate)**, **\<A\>((A&, V)&#8594;( ) aggregate)**, **(Type\<I\>&#8594;A builder, (A&, V)&#8594;( ) aggregate)**: `aggregate` the values into `aggregator`. |
| max <br> min | I &#8658; optional\<I\> or <br> I &#8658; Reference\<I\> | Find the max / min value from the inputs. Return `nullopt` if no input. <br> **.ref( )**: requires return value to be a reference to the input. |
| sum <br> avg | I &#8658; O | Calculate the sum / average of the inputs. <br> Return `nullopt` if no input or default value. <br> **.init(O v)**: set the initial value for summation |
| head <br> last | I &#8658; optional\<I\> | Return the first / last input. <br> Return `nullopt` if no input. |
| count | I &#8658; int64 | Return the number of inputs |
| to | I &#8658; Container or <br> I &#8658; ( ) | Insert each input to a new or existing container by `insert`, `push_back`, `push` or `emplace`. <br> **\<Container\>( )**, **(Container c)**, **(Type\<I\>&#8594;Container)**: specify the container to store the inputs. <br> **(Container& c)**: forward the inputs to an existing container. <br> **.reserve(int64)**: reverse capacity for the container, if possible. <br> **.by_move( )**: insert the inputs into the container by `move` (default: by copy).|
| print <br> println| I &#8658; ( ) | Forward each input to an output stream. <br> **(Str start, Str delimiter, Str end)**: `start` / `end` is inserted before / after the first / last input while `delimiter` is inserted between any two inputs (default: "[", ", ", "]" for `print`; "", "\n", "\n" for `println`). <br> **.to(ostream& os)**: output to `os` (default: `cout`). <br> **.format((ostream&, I)&#8594;( ) formatter)**: define how to output an input to the output stream. |

| Optimized Pipeline | Description |
| :---- | :---- |
| sort \| to &#8658; sort.buffer | The buffer of `sort` operator will be used as the output of `to` if type matches. |
| reverse | Reverson is pushed upward to a reversable operator to carry out, if possible. |

| Not Optimized Pipeline | Why |
| :---- | :---- |
| sort \| last &#8655; max <br> sort \| head &#8655; min | This can be easily replaced by the user. |
| iterate(c) \| count &#8655; c.size( ) | This can be easily replaced by the user. |
| sort \| distinct &#8655; sort \| unique | User has better knowledge to tell if `unique` is better than `distinct`. |
