
#include <math.h>

#include "benchmark.hpp"
#include "spdataparallel.hpp"
#include "sprandgen.hpp"

namespace sptl {

double ddotprod(const double* row, const double* vec, size_type n) {
  return level1::reducei(vec, vec + n, 0.0, [&] (double x, double y) {
    return x + y;
  }, [&] (size_type i, const double&) {
    return row[i] * vec[i];
  });
}

double ddotprod(const double* mtx, const double* vec, size_type n, size_type i) {
  ddotprod(mtx + (i * n), vec, n);
}
    
void dmdvmult(const double* mtx, const double* vec, double* dest, size_type n) {
  auto comp_rng = [&] (size_type lo, size_type hi) {
    return (hi - lo) * n;
  };
  parallel_for((size_type)0, n, comp_rng, [&] (size_type i) {
    dest[i] = ddotprod(mtx, vec, n, i);
  });
}

// this version is the same as the one above, except it specifies for the parallel
// loop an alternative sequential body
void dmdvmult_alt(const double* mtx, const double* vec, double* dest, size_type n) {
  auto comp_rng = [&] (size_type lo, size_type hi) {
    return (hi - lo) * n;
  };
  parallel_for((size_type)0, n, comp_rng, [&] (size_type i) {
    dest[i] = ddotprod(mtx, vec, n, i);
  }, [&] (size_type lo, size_type hi) {
    for (size_type i = lo; i < hi; i++) {
      double dotp = 0.0;
      for (size_type j = 0; j < n; j++) {
        dotp += mtx[i*n+j] * vec[j];
      }
      dest[i] = dotp;
    }
  });
}

void ex(sptl::bench::measured_type measured) {
  size_type n = deepsea::cmdline::parse_or_default_int("n", 10);
  auto gen = [&] (size_type i, unsigned int h) {
    return 0.1 * (double)h;
  };
  parray<double> mtx = gen_parray<double, decltype(gen)>(n * n, gen);
  parray<double> vec = gen_parray<double, decltype(gen)>(n, gen);
  parray<double> result(n);
  deepsea::cmdline::dispatcher d;
  d.add("simple", [&] {
    measured([&] {
      dmdvmult(mtx.cbegin(), vec.cbegin(), result.begin(), n);
    });
  });
  d.add("alternative", [&] {
    measured([&] {
      dmdvmult_alt(mtx.cbegin(), vec.cbegin(), result.begin(), n);
    });
  });
  d.dispatch_or_default("algorithm", "simple");
}

} // end namespace

int main(int argc, char** argv) {
  sptl::bench::launch(argc, argv, [&] (sptl::bench::measured_type measured) {
    sptl::ex(measured);
  });
  return 0;
}
