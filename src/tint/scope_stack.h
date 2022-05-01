// Copyright 2020 The Tint Authors.  //
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

#ifndef SRC_TINT_SCOPE_STACK_H_
#define SRC_TINT_SCOPE_STACK_H_

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/symbol.h"

namespace tint {

/// Used to store a stack of scope information.
/// The stack starts with a global scope which can not be popped.
template <class T>
class ScopeStack {
  public:
    /// Constructor
    ScopeStack() {
        // Push global bucket
        stack_.push_back({});
    }
    /// Copy Constructor
    ScopeStack(const ScopeStack&) = default;
    ~ScopeStack() = default;

    /// Push a new scope on to the stack
    void Push() { stack_.push_back({}); }

    /// Pop the scope off the top of the stack
    void Pop() {
        if (stack_.size() > 1) {
            stack_.pop_back();
        }
    }

    /// Assigns the value into the top most scope of the stack.
    /// @param symbol the symbol of the value
    /// @param val the value
    /// @returns the old value if there was an existing symbol at the top of the
    /// stack, otherwise the zero initializer for type T.
    T Set(const Symbol& symbol, T val) {
        std::swap(val, stack_.back()[symbol]);
        return val;
    }

    /// Retrieves a value from the stack
    /// @param symbol the symbol to look for
    /// @returns the value, or the zero initializer if the value was not found
    T Get(const Symbol& symbol) const {
        for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter) {
            auto& map = *iter;
            auto val = map.find(symbol);
            if (val != map.end()) {
                return val->second;
            }
        }

        return T{};
    }

  private:
    std::vector<std::unordered_map<Symbol, T>> stack_;
};

}  // namespace tint

#endif  // SRC_TINT_SCOPE_STACK_H_
