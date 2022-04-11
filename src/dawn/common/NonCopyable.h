// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_COMMON_NONCOPYABLE_H_
#define SRC_DAWN_COMMON_NONCOPYABLE_H_

// A base class to make a class non-copyable.
class NonCopyable {
  protected:
    constexpr NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;

  private:
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
};

// A base class to make a class non-movable.
class NonMovable : NonCopyable {
  protected:
    constexpr NonMovable() = default;
    ~NonMovable() = default;

  private:
    NonMovable(NonMovable&&) = delete;
    void operator=(NonMovable&&) = delete;
};

#endif  // SRC_DAWN_COMMON_NONCOPYABLE_H_
