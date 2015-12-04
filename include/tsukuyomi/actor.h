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
#ifndef __tsukuyomi_actor__
#define __tsukuyomi_actor__

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "channel.h"
#include "simple_concurrent_queue.h"

namespace tsukuyomi {

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

#endif  // __tsukuyomi_actor__
