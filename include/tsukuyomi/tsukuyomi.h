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
    if (empty()) return nullptr;

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
        _fn(_q.dequeue());
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

template <class SelfType>
class Actor {
 public:
  using AsyncFunction = std::function<void(SelfType &)>;
  using MailboxFunction = std::function<void(const std::string &)>;

  Actor() = default;
  virtual ~Actor() = default;

  void async(const AsyncFunction &fn) {
    lazy_init();
    _ach.enqueue(std::make_shared<AsyncFunction>(fn));
  }

  void post_mailbox(const std::string &obj) {
    lazy_init();
    _mbch.enqueue(std::make_shared<std::string>(obj));
  }

  void receive_mailbox(const MailboxFunction &fn) {
    lazy_init();
    _mb_fn = fn;
  }

 private:
  /// async channel
  Channel<AsyncFunction> _ach;

  // mailbox channel
  Channel<std::string> _mbch;

  volatile bool _initialized = false;

  /// mailbox queue
  SimpleConcurrentQueue<std::string> _mb;

  MailboxFunction _mb_fn = nullptr;

  void lazy_init() {
    if (!_initialized) {
      _ach.on([&](std::shared_ptr<AsyncFunction> obj) {
        (*obj)(dynamic_cast<SelfType &>(*this));
      });

      _mbch.on([&](std::shared_ptr<std::string> obj) { _mb_fn(*obj); });

      _initialized = true;
    }
  }

  Actor(Actor const &) = delete;
  Actor(Actor &&) = delete;
  Actor &operator=(Actor const &) = delete;
  Actor &operator=(Actor &&) = delete;
};
}

#endif