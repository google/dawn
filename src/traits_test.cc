// Copyright 2020 The Tint Authors.
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

#include "src/traits.h"

#include "gtest/gtest.h"

namespace tint {
namespace traits {

namespace {
struct S {};
void F1(S) {}
void F3(int, S, float) {}
}  // namespace

TEST(ParamType, Function) {
  F1({});        // Avoid unused method warning
  F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParamTypeT<decltype(&F1), 0>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&F3), 0>, int>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&F3), 1>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&F3), 2>, float>::value, "");
}

TEST(ParamType, Method) {
  class C {
   public:
    void F1(S) {}
    void F3(int, S, float) {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParamTypeT<decltype(&C::F1), 0>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 0>, int>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 1>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 2>, float>::value,
                "");
}

TEST(ParamType, ConstMethod) {
  class C {
   public:
    void F1(S) const {}
    void F3(int, S, float) const {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParamTypeT<decltype(&C::F1), 0>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 0>, int>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 1>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 2>, float>::value,
                "");
}

TEST(ParamType, StaticMethod) {
  class C {
   public:
    static void F1(S) {}
    static void F3(int, S, float) {}
  };
  C::F1({});        // Avoid unused method warning
  C::F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParamTypeT<decltype(&C::F1), 0>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 0>, int>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 1>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(&C::F3), 2>, float>::value,
                "");
}

TEST(ParamType, FunctionLike) {
  using F1 = std::function<void(S)>;
  using F3 = std::function<void(int, S, float)>;
  static_assert(std::is_same<ParamTypeT<F1, 0>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<F3, 0>, int>::value, "");
  static_assert(std::is_same<ParamTypeT<F3, 1>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<F3, 2>, float>::value, "");
}

TEST(ParamType, Lambda) {
  auto l1 = [](S) {};
  auto l3 = [](int, S, float) {};
  static_assert(std::is_same<ParamTypeT<decltype(l1), 0>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(l3), 0>, int>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(l3), 1>, S>::value, "");
  static_assert(std::is_same<ParamTypeT<decltype(l3), 2>, float>::value, "");
}

}  // namespace traits
}  // namespace tint
