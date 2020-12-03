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

#include "src/ast/traits.h"

#include <functional>

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace traits {

namespace {
struct S {};
void F(S) {}
}  // namespace

TEST(FirstParamType, Function) {
  F({});  // Avoid unused method warning
  static_assert(std::is_same<FirstParamTypeT<decltype(&F)>, S>::value, "");
}

TEST(FirstParamType, Method) {
  class C {
   public:
    void f(S) {}
  };
  C().f({});  // Avoid unused method warning
  static_assert(std::is_same<FirstParamTypeT<decltype(&C::f)>, S>::value, "");
}

TEST(FirstParamType, ConstMethod) {
  class C {
   public:
    void f(S) const {}
  };
  C().f({});  // Avoid unused method warning
  static_assert(std::is_same<FirstParamTypeT<decltype(&C::f)>, S>::value, "");
}

TEST(FirstParamType, StaticMethod) {
  class C {
   public:
    static void f(S) {}
  };
  C().f({});  // Avoid unused method warning
  static_assert(std::is_same<FirstParamTypeT<decltype(&C::f)>, S>::value, "");
}

TEST(FirstParamType, FunctionLike) {
  static_assert(std::is_same<FirstParamTypeT<std::function<void(S)>>, S>::value,
                "");
}

TEST(FirstParamType, Lambda) {
  auto l = [](S) {};
  static_assert(std::is_same<FirstParamTypeT<decltype(l)>, S>::value, "");
}

}  // namespace traits
}  // namespace ast
}  // namespace tint
