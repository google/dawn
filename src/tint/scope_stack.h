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

#include <utility>

#include "src/tint/symbol.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/vector.h"

namespace tint {

/// Used to store a stack of scope information.
/// The stack starts with a global scope which can not be popped.
template <class K, class V>
class ScopeStack {
  public:
    /// Push a new scope on to the stack
    void Push() { stack_.Push({}); }

    /// Pop the scope off the top of the stack
    void Pop() {
        if (stack_.Length() > 1) {
            stack_.Pop();
        }
    }

    /// Assigns the value into the top most scope of the stack.
    /// @param key the key of the value
    /// @param val the value
    /// @returns the old value if there was an existing key at the top of the
    /// stack, otherwise the zero initializer for type T.
    V Set(const K& key, V val) {
        auto& back = stack_.Back();
        if (auto el = back.Find(key)) {
            std::swap(val, *el);
            return val;
        }
        back.Add(key, val);
        return {};
    }

    /// Retrieves a value from the stack
    /// @param key the key to look for
    /// @returns the value, or the zero initializer if the value was not found
    V Get(const K& key) const {
        for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter) {
            if (auto val = iter->Find(key)) {
                return *val;
            }
        }

        return V{};
    }

    /// Return the top scope of the stack.
    /// @returns the top scope of the stack
    const utils::Hashmap<K, V, 4>& Top() const { return stack_.Back(); }

    /// Clear the scope stack.
    void Clear() {
        stack_.Clear();
        stack_.Push({});
    }

  private:
    utils::Vector<utils::Hashmap<K, V, 4>, 8> stack_ = {{}};
};

}  // namespace tint

#endif  // SRC_TINT_SCOPE_STACK_H_
