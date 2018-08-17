#pragma once

#include <ctime>
#include <algorithm>
#include <boost/random.hpp>

using namespace boost;

/** This class exists to use "random_shuffle".
 */
class Random
{
 public:
  /** I don't know what this function does.
   * I heard this is something called the function object.
   */
  unsigned int operator()(unsigned int max)
  {
    mt19937 gen(static_cast<unsigned long>(time(0))); //ここに置くのは正直よくない
    uniform_smallint<> dst(0, max - 1);
    variate_generator<mt19937&, uniform_smallint<> > rand(gen, dst);
    return rand();
  }
};
