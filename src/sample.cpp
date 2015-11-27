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

#include <iostream>
#include <tsukuyomi/tsukuyomi.h>

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