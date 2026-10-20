#include <iostream>
#include <math.h>
#include "benchmark.hpp"

/***********************************************************************/

namespace par = pasl::sched::native;

long cutoff = 0;

void atomic_add(long* ptr, long value) {
  long old_value;
  long new_value;

  do {
    old_value = *ptr;
    new_value = old_value + value;
  } while (!__sync_bool_compare_and_swap(ptr, old_value, new_value));
}

long fib(long n) {
  long result = 0;

  if (n < 2) {
    result = n;
  } else {
    par::fork2([&] {
      atomic_add(&result, fib(n-1));
    }, [&] {
      atomic_add(&result, fib(n-2));
    });
  }

  return result;
}

int main(int argc, char** argv) {
  long result = 0;
  long n = 0;

  auto init = [&] {
    cutoff = (long)pasl::util::cmdline::parse_or_default_int("cutoff", 25);
    n = (long)pasl::util::cmdline::parse_or_default_int("n", 24);
  };

  auto run = [&] (bool sequential) {
    result = fib(n);
  };

  auto output = [&] {
    std::cout << "n result " << n << " " << result << std::endl;
  };

  auto destroy = [&] {
    ;
  };

  pasl::sched::launch(argc, argv, init, run, output, destroy);
  return 0;
}
