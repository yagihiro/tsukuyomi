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
#ifndef __tsukuyomi_tsukuyomi__
#define __tsukuyomi_tsukuyomi__

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace tsukuyomi {

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

  std::shared_ptr<T> dequeue() {
    std::lock_guard<std::mutex> lock(_m);
    auto obj = _q.front();
    _q.pop();
    return std::move(obj);
  }
  bool empty() const { return _q.empty(); }

 private:
  Container _q;
  std::mutex _m;
};

template <typename SelfType>
class Actor {
 public:
  using AsyncFunction = std::function<void(SelfType &)>;
  using MailboxFunction = std::function<void(const std::string &)>;

  Actor() : _th() {}

  virtual ~Actor() {
    terminate();
    _th.join();
  }

  void async(const AsyncFunction &fn) {
    init_thread();

    {
      std::lock_guard<std::mutex> lock(_m);
      _aq.enqueue(std::make_shared<AsyncFunction>(fn));
    }
  }

  void post_mailbox(const std::string &obj) {
    init_thread();

    {
      std::lock_guard<std::mutex> lock(_m);
      _mb.enqueue(std::make_shared<std::string>(obj));
    }
  }

  void receive_mailbox(const MailboxFunction &fn) {
    init_thread();

    _mb_fn = fn;
  }

  void terminate() { _requested_termination = true; }

 private:
  /// this actor thread
  std::thread _th;

  /// this actor mutex
  std::mutex _m;

  /// true if an user requested termination
  volatile bool _requested_termination = false;

  /// async queue
  SimpleConcurrentQueue<AsyncFunction> _aq;

  /// mailbox queue
  SimpleConcurrentQueue<std::string> _mb;

  MailboxFunction _mb_fn = nullptr;

  void init_thread() {
    std::lock_guard<std::mutex> lock(_m);
    if (_th.get_id() == std::thread::id()) {
      _th = std::thread([this] { run(); });
    }
  }

  void run() {
    while (!_requested_termination) {
      if (!_aq.empty()) {
        auto a = _aq.dequeue();
        (*a)(dynamic_cast<SelfType &>(*this));
      }

      if (_mb_fn) {
        auto obj = _mb.dequeue();
        _mb_fn(*obj);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  Actor(Actor const &) = delete;
  Actor(Actor &&) = delete;
  Actor &operator=(Actor const &) = delete;
  Actor &operator=(Actor &&) = delete;
};
}

#endif