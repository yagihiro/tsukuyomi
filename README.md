# tsukuyomi
TSUKUYOMI is an actor model library for C++11

# Getting started

```C++
#include <tsukuyomi/tsukuyomi.h>
#include <iostream>

class Sample : public tsukuyomi::Actor<Sample> {
 public:
  void echo() { std::cout << "ECHO" << std::endl; }
};

int main(int argc, const char *argv[]) {
  Sample s;

  std::cout << "AAA" << std::endl;

  s.async([&](Sample &self) { self.echo(); });

  std::cout << "BBB" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  std::cout << "CCC" << std::endl;

  /* =>
  AAA
  BBB
  ECHO
  CCC
  */

  return 0;
}

```

# Contributing

1. Fork it ( https://github.com/[my-github-username]/tsukuyomi/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request