/*
 The MIT License (MIT)

 Copyright (c) 2015 Hiroki Yagita

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <tsukuyomi/tsukuyomi.h>
#include <iostream>

class Sample : public tsukuyomi::Actor<Sample> {
 public:
  void echo() { std::cout << "ECHO" << std::endl; }
};

class Echo {
 public:
  Echo() {
    std::cout << "Echo construct:" << reinterpret_cast<int *>(this)
              << std::endl;
  }
  ~Echo() {
    std::cout << "Echo destruct:" << reinterpret_cast<int *>(this) << std::endl;
  }
  std::string echo() const { return "Echo()"; }
};

void queue_sample() {
  SimpleConcurrentQueue<Echo> q;
  std::shared_ptr<Echo> echo = std::make_shared<Echo>();
  std::cout << q.empty() << std::endl;
  q.enqueue(echo);
  std::cout << q.empty() << std::endl;
  auto i = q.dequeue(std::chrono::minutes(1));
  std::cout << q.empty() << std::endl;
  std::cout << i->echo() << std::endl;
}

SimpleConcurrentQueue<int> _q;
void cq_test() {
  std::thread th1([]() {
    int i = 0;
    do {
      _q.enqueue(std::make_shared<int>(i));
    } while (i++ < 100000);
    std::cout << "Finished enqueueing" << std::endl;
  });
  std::thread th2([]() {
    int i = 0;
    do {
      auto value = _q.try_dequeue();
      if (value == nullptr) {
        i--;
        continue;
      }
      if (*value % 1000 == 0) std::cout << *value << " ";
    } while (i++ < 100000);
    std::cout << "Finished dequeueing" << std::endl;
  });
  th1.join();
  th2.join();
}

SimpleConcurrentQueue<int> _q2;
void cq_test2() {
  std::thread th1([]() {
    int i = 0;
    do {
      _q2.enqueue(std::make_shared<int>(i));
    } while (i++ < 100000);
    std::cout << "Finished enqueueing" << std::endl;
  });
  std::thread th2([]() {
    int i = 0;
    do {
      auto value = _q2.dequeue(std::chrono::milliseconds(1));
      if (value == nullptr) {
        i--;
        continue;
      }
      if (*value % 1000 == 0) std::cout << *value << " ";
    } while (i++ < 100000);
    std::cout << "Finished dequeueing" << std::endl;
  });
  th2.join();
  th1.join();
}

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

  queue_sample();
  cq_test();
  cq_test2();

  return 0;
}
