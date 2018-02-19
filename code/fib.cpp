
#include <math.h>

#include "spreduce.hpp"
#include "spio.hpp"

namespace sptl {

long fib_seq(long n){
  if (n < 2) {
    return n;
  } else { 
    return fib_seq (n - 1) + fib_seq (n - 2);
  }
}

double phi = (1 + sqrt(5)) / 2;

long fib_par(long n) {
  long result;
  spguard([&] { return pow(phi, n); }, [&] {
    if (n < 2) {
      result = fib_seq(n);
      return;
    }
    long a, b;
    fork2([&] { a = fib_par(n-1); },
          [&] { b = fib_par(n-2); });
    result = a + b;
  }, [&] {
    result = fib_seq(n);
  });
  return result;
}

void ex() {
  long n = deepsea::cmdline::parse_or_default_int("n", 10);
  long r = fib_par(n);
  std::cout << "result\t" << r << std::endl;
}

} // end namespace

int main(int argc, char** argv) {
  sptl::launch(argc, argv, [&] {
    sptl::ex();
  });
  return 0;
}
