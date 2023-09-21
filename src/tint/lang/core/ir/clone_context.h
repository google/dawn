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
            auto* cast = As<T>((*replacement)());
            TINT_ASSERT(cast);
            return cast;
        }
        auto* result = what->Clone(*this)->template As<T>();
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

    /// Registers the replacement of `what` with `with`
    /// @param what the value or instruction to replace
    /// @param with either a pointer to a replacement instruction, or a function with the signature
    /// `T*(T*)` used to build the replacement
    template <typename WHAT, typename WITH>
    void Replace(WHAT* what, WITH&& with) {
        using T = std::decay_t<WHAT>;
        using F = std::decay_t<WITH>;

        constexpr bool T_is_value = traits::IsTypeOrDerived<T, Value>;
        constexpr bool T_is_instruction = traits::IsTypeOrDerived<T, Instruction>;
        static_assert(T_is_value || T_is_instruction);

        constexpr bool F_is_pointer = std::is_pointer_v<F>;
        constexpr bool F_is_function = std::is_function_v<F>;
        static_assert(F_is_pointer || F_is_function);

        if constexpr (F_is_pointer) {
            replacements_.Add(what, [with]() { return with; });
        } else if constexpr (F_is_function) {
            static_assert(std::is_same_v<traits::ParameterType<F, 0>, T*>);
            static_assert(std::is_same_v<traits::ReturnType<F>, T*>);
            replacements_.Add(what, [what, with]() { return with(what); });
        }
    }

  private:
    Hashmap<CastableBase*, std::function<CastableBase*()>, 8> replacements_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_
