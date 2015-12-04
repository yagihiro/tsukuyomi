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
#ifndef __tsukuyomi_channel__
#define __tsukuyomi_channel__

#include <functional>
#include <memory>
#include <thread>
#include "simple_concurrent_queue.h"

namespace tsukuyomi {

template <class T>
class Channel {
 public:
  Channel() {
    _th = std::thread([this] { run(); });
  }
  ~Channel() {
    terminate();
    _th.join();
  }

  void terminate() { _requested_termination = true; }

  void on(const std::function<void(std::shared_ptr<T>)> &fn) { _fn = fn; }

  void enqueue(std::shared_ptr<T> obj) { _q.enqueue(obj); }

 private:
  void run() {
    while (!_requested_termination) {
      if (!_q.empty() && _fn) {
        _fn(_q.try_dequeue());
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  std::thread _th;
  SimpleConcurrentQueue<T> _q;

  /// true if an user requested termination
  volatile bool _requested_termination = false;

  std::function<void(std::shared_ptr<T>)> _fn = nullptr;
};
}

#endif  // __tsukuyomi_channel__
