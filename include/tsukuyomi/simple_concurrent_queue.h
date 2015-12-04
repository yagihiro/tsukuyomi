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
#ifndef __simple_concurrent_queue__
#define __simple_concurrent_queue__

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

template <class T, class Container = std::queue<std::shared_ptr<T>>>
class SimpleConcurrentQueue {
 public:
  using container_type = Container;
  using value_type = typename Container::value_type;
  using size_type = typename Container::size_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;

  SimpleConcurrentQueue() {}
  ~SimpleConcurrentQueue() = default;

  void enqueue(std::shared_ptr<T> obj) {
    std::lock_guard<std::mutex> lock(_m);
    _q.emplace(obj);
  }

  std::shared_ptr<T> try_dequeue() {
    if (empty()) return nullptr;

    std::lock_guard<std::mutex> lock(_m);
    auto obj = _q.front();
    _q.pop();
    return std::move(obj);
  }

  template <class Rep, class Period>
  std::shared_ptr<T> dequeue(
      const std::chrono::duration<Rep, Period> &timeout_duration) {
    auto start = std::chrono::steady_clock::now();

    do {
      if (!empty()) {
        std::lock_guard<std::mutex> lock(_m);
        auto obj = _q.front();
        _q.pop();
        return std::move(obj);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      auto current = std::chrono::steady_clock::now();
      auto duration = current - start;
      auto c = (timeout_duration - duration).count();
      if (c <= 0) {
        break;
      }
    } while (true);

    return nullptr;
  }

  bool empty() const { return _q.empty(); }

 private:
  Container _q;
  std::mutex _m;
};

#endif  // __simple_concurrent_queue__
