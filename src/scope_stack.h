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

#ifndef SRC_SCOPE_STACK_H_
#define SRC_SCOPE_STACK_H_

#include <unordered_map>
#include <vector>

#include "src/symbol.h"

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
  void push_scope() { stack_.push_back({}); }

  /// Pop the scope off the top of the stack
  void pop_scope() {
    if (stack_.size() > 1) {
      stack_.pop_back();
    }
  }

  /// Set a global variable in the stack
  /// @param symbol the symbol of the variable
  /// @param val the value
  void set_global(const Symbol& symbol, T val) { stack_[0][symbol] = val; }

  /// Sets variable into the top most scope of the stack
  /// @param symbol the symbol of the variable
  /// @param val the value
  void set(const Symbol& symbol, T val) { stack_.back()[symbol] = val; }

  /// Checks for the given `symbol` in the stack
  /// @param symbol the symbol to look for
  /// @returns true if the stack contains `symbol`
  bool has(const Symbol& symbol) const { return get(symbol, nullptr); }

  /// Retrieves a given variable from the stack
  /// @param symbol the symbol to look for
  /// @param ret where to place the value
  /// @returns true if the symbol was successfully found, false otherwise
  bool get(const Symbol& symbol, T* ret) const {
    return get(symbol, ret, nullptr);
  }

  /// Retrieves a given variable from the stack
  /// @param symbol the symbol to look for
  /// @param ret where to place the value
  /// @param is_global set true if the symbol references a global variable
  /// otherwise unchanged
  /// @returns true if the symbol was successfully found, false otherwise
  bool get(const Symbol& symbol, T* ret, bool* is_global) const {
    for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter) {
      auto& map = *iter;
      auto val = map.find(symbol);

      if (val != map.end()) {
        if (ret) {
          *ret = val->second;
        }
        if (is_global && iter == stack_.rend() - 1) {
          *is_global = true;
        }
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<std::unordered_map<Symbol, T>> stack_;
};

}  // namespace tint

#endif  // SRC_SCOPE_STACK_H_
