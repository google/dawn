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

#ifndef SRC_AST_CLONE_CONTEXT_H_
#define SRC_AST_CLONE_CONTEXT_H_

#include <functional>
#include <unordered_map>
#include <vector>

#include "src/ast/traits.h"
#include "src/castable.h"
#include "src/source.h"

namespace tint {
namespace ast {

class Module;

/// CloneContext holds the state used while cloning AST nodes and types.
class CloneContext {
 public:
  /// Constructor
  /// @param m the target module to clone into
  explicit CloneContext(Module* m);

  /// Destructor
  ~CloneContext();

  /// Clones the `Node` or `type::Type` `a` into the module #mod if `a` is not
  /// null. If `a` is null, then Clone() returns null. If `a` has been cloned
  /// already by this CloneContext then the same cloned pointer is returned.
  ///
  /// Clone() may use a function registered with ReplaceAll() to create a
  /// transformed version of the object. See ReplaceAll() for more information.
  ///
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param a the `Node` or `type::Type` to clone
  /// @return the cloned node
  template <typename T>
  T* Clone(T* a) {
    // If the input is nullptr, there's nothing to clone - just return nullptr.
    if (a == nullptr) {
      return nullptr;
    }

    // See if we've already cloned this object - if we have return the
    // previously cloned pointer.
    // If we haven't cloned this before, try cloning using a replacer transform.
    if (auto* c = LookupOrTransform(a)) {
      return static_cast<T*>(c);
    }

    // First time clone and no replacer transforms matched.
    // Clone with T::Clone().
    auto* c = a->Clone(this);
    cloned_.emplace(a, c);
    return static_cast<T*>(c);
  }

  /// Clones the `Source` `s` into `mod`
  /// TODO(bclayton) - Currently this 'clone' is a shallow copy. If/when
  /// `Source.File`s are owned by the `Module` this should make a copy of the
  /// file.
  /// @param s the `Source` to clone
  /// @return the cloned source
  Source Clone(const Source& s) { return s; }

  /// Clones each of the elements of the vector `v` into the module #mod->
  /// @param v the vector to clone
  /// @return the cloned vector
  template <typename T>
  std::vector<T> Clone(const std::vector<T>& v) {
    std::vector<T> out;
    out.reserve(v.size());
    for (auto& el : v) {
      out.emplace_back(Clone(el));
    }
    return out;
  }

  /// ReplaceAll() registers `replacer` to be called whenever the Clone() method
  /// is called with a type that matches (or derives from) the type of the first
  /// parameter of `replacer`.
  ///
  /// `replacer` must be function-like with the signature: `T* (T*)`, where `T`
  /// is a type deriving from CastableBase.
  ///
  /// If `replacer` returns a nullptr then Clone() will attempt the next
  /// registered replacer function that matches the object type. If no replacers
  /// match the object type, or all returned nullptr then Clone() will call
  /// `T::Clone()` to clone the object.
  ///
  /// Example:
  ///
  /// ```
  ///   // Replace all ast::UintLiterals with the number 42
  ///   CloneCtx ctx(mod);
  ///   ctx.ReplaceAll([&] (ast::UintLiteral* in) {
  ///     return ctx.mod->create<ast::UintLiteral>(ctx.Clone(in->source()),
  ///                                              ctx.Clone(in->type()), 42);
  ///   });
  ///   auto* out = ctx.Clone(tree);
  /// ```
  ///
  /// @param replacer a function or function-like object with the signature
  ///        `T* (T*)`, where `T` derives from CastableBase
  template <typename F>
  void ReplaceAll(F replacer) {
    using TPtr = traits::ParamTypeT<F, 0>;
    using T = typename std::remove_pointer<TPtr>::type;
    transforms_.emplace_back([=](CastableBase* in) {
      auto* in_as_t = in->As<T>();
      return in_as_t != nullptr ? replacer(in_as_t) : nullptr;
    });
  }

  /// The target module to clone into.
  Module* const mod;

 private:
  using Transform = std::function<CastableBase*(CastableBase*)>;

  /// LookupOrTransform is the template-independent logic of Clone().
  /// This is outside of Clone() to reduce the amount of template-instantiated
  /// code.
  CastableBase* LookupOrTransform(CastableBase* a) {
    // Have we seen this object before? If so, return the previously cloned
    // version instead of making yet another copy.
    auto it = cloned_.find(a);
    if (it != cloned_.end()) {
      return it->second;
    }

    // Attempt to clone using the registered replacer functions.
    for (auto& f : transforms_) {
      if (CastableBase* c = f(a)) {
        cloned_.emplace(a, c);
        return c;
      }
    }

    // No luck, Clone() will have to call T::Clone().
    return nullptr;
  }

  std::unordered_map<CastableBase*, CastableBase*> cloned_;
  std::vector<Transform> transforms_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CLONE_CONTEXT_H_
