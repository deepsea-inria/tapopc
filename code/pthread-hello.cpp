
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  static constexpr
  int nb_threads = 8;
  for (int i = 0; i < nb_threads; i++) {
    std::cout << "main: creating thread 00" << i << std::endl;
    auto t = std::thread([=] {
      std::cout << "Hello world! It is me, 00" << i << std::endl;
    });
    t.detach();
  }
  return 0;
}
