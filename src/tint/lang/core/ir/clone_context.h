// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_
#define SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/traits/traits.h"

namespace tint::core::ir {
class Block;
class Instruction;
class Module;
class Value;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Constant in the IR.
class CloneContext {
  public:
    /// @param module the IR module
    explicit CloneContext(Module& module);

    /// The IR module
    Module& ir;

    /// Performs a clone of @p what.
    /// @param what the item to clone
    /// @return the cloned item
    template <typename T>
    T* Clone(T* what) {
        if (auto replacement = replacements_.Get(what)) {
            return (*replacement)->template As<T>();
        }
        T* result = what->Clone(*this);
        Replace(what, result);
        return result;
    }

    /// Performs a clone of all the elements in @p what.
    /// @param what the elements to clone
    /// @return the cloned elements
    template <size_t N, typename T>
    Vector<T*, N> Clone(Slice<T* const> what) {
        return Transform<N>(what, [&](T* const p) { return Clone(p); });
    }

    /// Performs a clone of all the elements in @p what.
    /// @param what the elements to clone
    /// @return the cloned elements
    template <size_t N, typename T>
    Vector<T*, N> Clone(Slice<T*> what) {
        return Transform<N>(what, [&](T* p) { return Clone(p); });
    }

    /// Performs a clone of all the elements in @p what.
    /// @param what the elements to clone
    /// @return the cloned elements
    template <size_t N, typename T>
    Vector<T*, N> Clone(Vector<T*, N> what) {
        return Transform(what, [&](T* p) { return Clone(p); });
    }

    /// Obtains the (potentially) remapped pointer to @p what
    /// @param what the item
    /// @return the cloned item for @p what, or the original pointer if @p what has not been cloned.
    template <typename T>
    T* Remap(T* what) {
        if (auto replacement = replacements_.Get(what)) {
            return (*replacement)->template As<T>();
        }
        return what;
    }

    /// Obtains the (potentially) remapped pointer of all the elements in @p what.
    /// @param what the item
    /// @return the remapped elements
    template <size_t N, typename T>
    Vector<T*, N> Remap(Slice<T* const> what) {
        return Transform<N>(what, [&](T* const p) { return Remap(p); });
    }

    /// Obtains the (potentially) remapped pointer of all the elements in @p what.
    /// @param what the item
    /// @return the remapped elements
    template <size_t N, typename T>
    Vector<T*, N> Remap(Slice<T*> what) {
        return Transform<N>(what, [&](T* p) { return Remap(p); });
    }

    /// Obtains the (potentially) remapped pointer of all the elements in @p what.
    /// @param what the item
    /// @return the remapped elements
    template <size_t N, typename T>
    Vector<T*, N> Remap(Vector<T*, N> what) {
        return Transform(what, [&](T* p) { return Remap(p); });
    }

    /// Registers the replacement of `what` with `with`
    /// @param what the value or instruction to replace
    /// @param with a pointer to a replacement value or instruction
    template <typename WHAT, typename WITH>
    void Replace(WHAT* what, WITH* with) {
        static_assert(traits::IsTypeOrDerived<WHAT, ir::Instruction> ||
                      traits::IsTypeOrDerived<WHAT, ir::Value>);
        static_assert(traits::IsTypeOrDerived<WITH, WHAT>);
        TINT_ASSERT(with);
        replacements_.Add(what, with);
    }

  private:
    Hashmap<CastableBase*, CastableBase*, 8> replacements_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_
