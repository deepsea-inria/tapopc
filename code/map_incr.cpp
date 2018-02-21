
#include <math.h>

#include "benchmark.hpp"
#include "spdataparallel.hpp"

namespace sptl {

void map_incr(const int* source, int* dest, size_type n) {
  parallel_for((size_type)0, n, [&] (size_type i) {
    dest[i] = source[i] + 1;
  });
}

void ex(sptl::bench::measured_type measured) {
  size_type n = deepsea::cmdline::parse_or_default_int("n", 1000000);
  parray<int> a(n);
  parray<int> b(n);
  measured([&] {
    map_incr(a.cbegin(), b.begin(), n);
  });
}

} // end namespace

int main(int argc, char** argv) {
  sptl::bench::launch(argc, argv, [&] (sptl::bench::measured_type measured) {
    sptl::ex(measured);
  });
  return 0;
}
