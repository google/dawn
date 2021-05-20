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

#ifndef SRC_CLONE_CONTEXT_H_
#define SRC_CLONE_CONTEXT_H_

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/castable.h"
#include "src/debug.h"
#include "src/symbol.h"
#include "src/traits.h"

namespace tint {

// Forward declarations
class CloneContext;
class Program;
class ProgramBuilder;
namespace ast {
class FunctionList;
class Node;
}  // namespace ast

ProgramID ProgramIDOf(const Program*);
ProgramID ProgramIDOf(const ast::Node*);

/// Cloneable is the base class for all objects that can be cloned
class Cloneable : public Castable<Cloneable> {
 public:
  /// Performs a deep clone of this object using the CloneContext `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned object
  virtual Cloneable* Clone(CloneContext* ctx) const = 0;
};

/// @returns an invalid ProgramID
inline ProgramID ProgramIDOf(const Cloneable*) {
  return ProgramID();
}

/// CloneContext holds the state used while cloning AST nodes.
class CloneContext {
  /// ParamTypeIsPtrOf<F, T>::value is true iff the first parameter of
  /// F is a pointer of (or derives from) type T.
  template <typename F, typename T>
  using ParamTypeIsPtrOf = traits::IsTypeOrDerived<
      typename std::remove_pointer<traits::ParamTypeT<F, 0>>::type,
      T>;

 public:
  /// SymbolTransform is a function that takes a symbol and returns a new
  /// symbol.
  using SymbolTransform = std::function<Symbol(Symbol)>;

  /// Constructor for cloning objects from `from` into `to`.
  /// @param to the target ProgramBuilder to clone into
  /// @param from the source Program to clone from
  /// @param auto_clone_symbols clone all symbols in `from` before returning
  CloneContext(ProgramBuilder* to,
               Program const* from,
               bool auto_clone_symbols = true);

  /// Constructor for cloning objects from and to the ProgramBuilder `builder`.
  /// @param builder the ProgramBuilder
  explicit CloneContext(ProgramBuilder* builder);

  /// Destructor
  ~CloneContext();

  /// Clones the Node or sem::Type `a` into the ProgramBuilder #dst if `a` is
  /// not null. If `a` is null, then Clone() returns null. If `a` has been
  /// cloned already by this CloneContext then the same cloned pointer is
  /// returned.
  ///
  /// Clone() may use a function registered with ReplaceAll() to create a
  /// transformed version of the object. See ReplaceAll() for more information.
  ///
  /// If the CloneContext is cloning from a Program to a ProgramBuilder, then
  /// the Node or sem::Type `a` must be owned by the Program #src.
  ///
  /// @param a the `Node` or `sem::Type` to clone
  /// @return the cloned node
  template <typename T>
  T* Clone(T* a) {
    // If the input is nullptr, there's nothing to clone - just return nullptr.
    if (a == nullptr) {
      return nullptr;
    }

    if (src) {
      TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, a);
    }

    // Was Replace() called for this object?
    auto it = replacements_.find(a);
    if (it != replacements_.end()) {
      return CheckedCast<T>(it->second);
    }

    Cloneable* cloned = nullptr;

    // Attempt to clone using the registered replacer functions.
    auto& typeinfo = a->TypeInfo();
    for (auto& transform : transforms_) {
      if (!typeinfo.Is(*transform.typeinfo)) {
        continue;
      }
      cloned = transform.function(a);
      break;
    }

    if (!cloned) {
      // No transform for this type, or the transform returned nullptr.
      // Clone with T::Clone().
      cloned = a->Clone(this);
    }

    auto* out = CheckedCast<T>(cloned);

    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(dst, out);

    return out;
  }

  /// Clones the Node or sem::Type `a` into the ProgramBuilder #dst if `a` is
  /// not null. If `a` is null, then Clone() returns null. If `a` has been
  /// cloned already by this CloneContext then the same cloned pointer is
  /// returned.
  ///
  /// Unlike Clone(), this method does not invoke or use any transformations
  /// registered by ReplaceAll().
  ///
  /// If the CloneContext is cloning from a Program to a ProgramBuilder, then
  /// the Node or sem::Type `a` must be owned by the Program #src.
  ///
  /// @param a the `Node` or `sem::Type` to clone
  /// @return the cloned node
  template <typename T>
  T* CloneWithoutTransform(T* a) {
    // If the input is nullptr, there's nothing to clone - just return nullptr.
    if (a == nullptr) {
      return nullptr;
    }

    if (src) {
      TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, a);
    }

    // Have we seen this object before? If so, return the previously cloned
    // version instead of making yet another copy.
    auto it = replacements_.find(a);
    if (it != replacements_.end()) {
      return CheckedCast<T>(it->second);
    }

    // First time clone and no replacer transforms matched.
    // Clone with T::Clone().
    auto* c = a->Clone(this);
    replacements_.emplace(a, c);
    return CheckedCast<T>(c);
  }

  /// Clones the Source `s` into #dst
  /// TODO(bclayton) - Currently this 'clone' is a shallow copy. If/when
  /// `Source.File`s are owned by the Program this should make a copy of the
  /// file.
  /// @param s the `Source` to clone
  /// @return the cloned source
  Source Clone(const Source& s) const { return s; }

  /// Clones the Symbol `s` into #dst
  ///
  /// The Symbol `s` must be owned by the Program #src.
  ///
  /// @param s the Symbol to clone
  /// @return the cloned source
  Symbol Clone(Symbol s);

  /// Clones each of the elements of the vector `v` into the ProgramBuilder
  /// #dst.
  ///
  /// All the elements of the vector `v` must be owned by the Program #src.
  ///
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

  /// Clones each of the elements of the vector `v` into the ProgramBuilder
  /// #dst, inserting any additional elements into the list that were registered
  /// with calls to InsertBefore().
  ///
  /// All the elements of the vector `v` must be owned by the Program #src.
  ///
  /// @param v the vector to clone
  /// @return the cloned vector
  template <typename T>
  std::vector<T*> Clone(const std::vector<T*>& v) {
    std::vector<T*> out;
    out.reserve(v.size());

    auto list_transform_it = list_transforms_.find(&v);
    if (list_transform_it != list_transforms_.end()) {
      const auto& transforms = list_transform_it->second;
      for (auto& el : v) {
        auto insert_before_it = transforms.insert_before_.find(el);
        if (insert_before_it != transforms.insert_before_.end()) {
          for (auto insert : insert_before_it->second) {
            out.emplace_back(CheckedCast<T>(insert));
          }
        }
        if (transforms.remove_.count(el) == 0) {
          out.emplace_back(Clone(el));
        }
        auto insert_after_it = transforms.insert_after_.find(el);
        if (insert_after_it != transforms.insert_after_.end()) {
          for (auto insert : insert_after_it->second) {
            out.emplace_back(CheckedCast<T>(insert));
          }
        }
      }
    } else {
      for (auto& el : v) {
        out.emplace_back(Clone(el));
      }
    }
    return out;
  }

  /// Clones each of the elements of the vector `v` into the ProgramBuilder
  /// #dst.
  ///
  /// All the elements of the vector `v` must be owned by the Program #src.
  ///
  /// @param v the vector to clone
  /// @return the cloned vector
  ast::FunctionList Clone(const ast::FunctionList& v);

  /// ReplaceAll() registers `replacer` to be called whenever the Clone() method
  /// is called with a Cloneable type that matches (or derives from) the type of
  /// the single parameter of `replacer`.
  /// The returned Cloneable of `replacer` will be used as the replacement for
  /// all references to the object that's being cloned. This returned Cloneable
  /// must be owned by the Program #dst.
  ///
  /// `replacer` must be function-like with the signature: `T* (T*)`
  ///  where `T` is a type deriving from Cloneable.
  ///
  /// If `replacer` returns a nullptr then Clone() will call `T::Clone()` to
  /// clone the object.
  ///
  /// Example:
  ///
  /// ```
  ///   // Replace all ast::UintLiterals with the number 42
  ///   CloneCtx ctx(&out, in);
  ///   ctx.ReplaceAll([&] (ast::UintLiteral* l) {
  ///       return ctx->dst->create<ast::UintLiteral>(
  ///           ctx->Clone(l->source()),
  ///           ctx->Clone(l->type()),
  ///           42);
  ///     });
  ///   ctx.Clone();
  /// ```
  ///
  /// @warning a single handler can only be registered for any given type.
  /// Attempting to register two handlers for the same type will result in an
  /// ICE.
  /// @warning The replacement object must be of the correct type for all
  /// references of the original object. A type mismatch will result in an
  /// assertion in debug builds, and undefined behavior in release builds.
  /// @param replacer a function or function-like object with the signature
  ///        `T* (T*)`, where `T` derives from Cloneable
  /// @returns this CloneContext so calls can be chained
  template <typename F>
  traits::EnableIf<ParamTypeIsPtrOf<F, Cloneable>::value, CloneContext>&
  ReplaceAll(F&& replacer) {
    using TPtr = traits::ParamTypeT<F, 0>;
    using T = typename std::remove_pointer<TPtr>::type;
    for (auto& transform : transforms_) {
      if (transform.typeinfo->Is(TypeInfo::Of<T>()) ||
          TypeInfo::Of<T>().Is(*transform.typeinfo)) {
        TINT_ICE(Diagnostics())
            << "ReplaceAll() called with a handler for type "
            << TypeInfo::Of<T>().name
            << " that is already handled by a handler for type "
            << transform.typeinfo->name;
        return *this;
      }
    }
    CloneableTransform transform;
    transform.typeinfo = &TypeInfo::Of<T>();
    transform.function = [=](Cloneable* in) { return replacer(in->As<T>()); };
    transforms_.emplace_back(std::move(transform));
    return *this;
  }

  /// ReplaceAll() registers `replacer` to be called whenever the Clone() method
  /// is called with a Symbol.
  /// The returned symbol of `replacer` will be used as the replacement for
  /// all references to the symbol that's being cloned. This returned Symbol
  /// must be owned by the Program #dst.
  /// @param replacer a function the signature `Symbol(Symbol)`.
  /// @warning a SymbolTransform can only be registered once. Attempting to
  /// register a SymbolTransform more than once will result in an ICE.
  /// @returns this CloneContext so calls can be chained
  CloneContext& ReplaceAll(const SymbolTransform& replacer) {
    if (symbol_transform_) {
      TINT_ICE(Diagnostics()) << "ReplaceAll(const SymbolTransform&) called "
                                 "multiple times on the same CloneContext";
      return *this;
    }
    symbol_transform_ = replacer;
    return *this;
  }

  /// Replace replaces all occurrences of `what` in #src with `with` in #dst
  /// when calling Clone().
  /// @param what a pointer to the object in #src that will be replaced with
  /// `with`
  /// @param with a pointer to the replacement object owned by #dst that will be
  /// used as a replacement for `what`
  /// @warning The replacement object must be of the correct type for all
  /// references of the original object. A type mismatch will result in an
  /// assertion in debug builds, and undefined behavior in release builds.
  /// @returns this CloneContext so calls can be chained
  template <typename WHAT, typename WITH>
  CloneContext& Replace(WHAT* what, WITH* with) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, what);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(dst, with);
    replacements_[what] = with;
    return *this;
  }

  /// Removes `object` from the cloned copy of `vector`.
  /// @param vector the vector in #src
  /// @param object a pointer to the object in #src that will be omitted from
  /// the cloned vector.
  /// @returns this CloneContext so calls can be chained
  template <typename T, typename OBJECT>
  CloneContext& Remove(const std::vector<T>& vector, OBJECT* object) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, object);
    if (std::find(vector.begin(), vector.end(), object) == vector.end()) {
      TINT_ICE(Diagnostics())
          << "CloneContext::Remove() vector does not contain object";
      return *this;
    }

    list_transforms_[&vector].remove_.emplace(object);
    return *this;
  }

  /// Inserts `object` before `before` whenever `vector` is cloned.
  /// @param vector the vector in #src
  /// @param before a pointer to the object in #src
  /// @param object a pointer to the object in #dst that will be inserted before
  /// any occurrence of the clone of `before`
  /// @returns this CloneContext so calls can be chained
  template <typename T, typename BEFORE, typename OBJECT>
  CloneContext& InsertBefore(const std::vector<T>& vector,
                             const BEFORE* before,
                             OBJECT* object) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, before);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(dst, object);
    if (std::find(vector.begin(), vector.end(), before) == vector.end()) {
      TINT_ICE(Diagnostics())
          << "CloneContext::InsertBefore() vector does not contain before";
      return *this;
    }

    auto& transforms = list_transforms_[&vector];
    auto& list = transforms.insert_before_[before];
    list.emplace_back(object);
    return *this;
  }

  /// Inserts `object` after `after` whenever `vector` is cloned.
  /// @param vector the vector in #src
  /// @param after a pointer to the object in #src
  /// @param object a pointer to the object in #dst that will be inserted after
  /// any occurrence of the clone of `after`
  /// @returns this CloneContext so calls can be chained
  template <typename T, typename AFTER, typename OBJECT>
  CloneContext& InsertAfter(const std::vector<T>& vector,
                            const AFTER* after,
                            OBJECT* object) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, after);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(dst, object);
    if (std::find(vector.begin(), vector.end(), after) == vector.end()) {
      TINT_ICE(Diagnostics())
          << "CloneContext::InsertAfter() vector does not contain after";
      return *this;
    }

    auto& transforms = list_transforms_[&vector];
    auto& list = transforms.insert_after_[after];
    list.emplace_back(object);
    return *this;
  }

  /// Clone performs the clone of the Program's AST nodes, types and symbols
  /// from #src to #dst. Semantic nodes are not cloned, as these will be rebuilt
  /// when the ProgramBuilder #dst builds its Program.
  void Clone();

  /// The target ProgramBuilder to clone into.
  ProgramBuilder* const dst;

  /// The source Program to clone from.
  Program const* const src;

 private:
  struct CloneableTransform {
    /// Constructor
    CloneableTransform();
    /// Copy constructor
    /// @param other the CloneableTransform to copy
    CloneableTransform(const CloneableTransform& other);
    /// Destructor
    ~CloneableTransform();

    // TypeInfo of the Cloneable that the transform operates on
    const TypeInfo* typeinfo;
    std::function<Cloneable*(Cloneable*)> function;
  };

  CloneContext(const CloneContext&) = delete;
  CloneContext& operator=(const CloneContext&) = delete;

  /// Cast `obj` from type `FROM` to type `TO`, returning the cast object.
  /// Reports an internal compiler error if the cast failed.
  template <typename TO, typename FROM>
  TO* CheckedCast(FROM* obj) {
    if (TO* cast = As<TO>(obj)) {
      return cast;
    }
    TINT_ICE(Diagnostics())
        << "Cloned object was not of the expected type\n"
        << "got:      " << (obj ? obj->TypeInfo().name : "<null>") << "\n"
        << "expected: " << TypeInfo::Of<TO>().name;
    return nullptr;
  }

  /// @returns the diagnostic list of #dst
  diag::List& Diagnostics() const;

  /// A vector of Cloneable*
  using CloneableList = std::vector<Cloneable*>;

  // Transformations to be applied to a list (vector)
  struct ListTransforms {
    /// Constructor
    ListTransforms();
    /// Destructor
    ~ListTransforms();

    /// A map of object in #src to omit when cloned into #dst.
    std::unordered_set<const Cloneable*> remove_;

    /// A map of object in #src to the list of cloned objects in #dst.
    /// Clone(const std::vector<T*>& v) will use this to insert the map-value
    /// list into the target vector before cloning and inserting the map-key.
    std::unordered_map<const Cloneable*, CloneableList> insert_before_;

    /// A map of object in #src to the list of cloned objects in #dst.
    /// Clone(const std::vector<T*>& v) will use this to insert the map-value
    /// list into the target vector after cloning and inserting the map-key.
    std::unordered_map<const Cloneable*, CloneableList> insert_after_;
  };

  /// A map of object in #src to their replacement in #dst
  std::unordered_map<const Cloneable*, Cloneable*> replacements_;

  /// A map of symbol in #src to their cloned equivalent in #dst
  std::unordered_map<Symbol, Symbol> cloned_symbols_;

  /// Cloneable transform functions registered with ReplaceAll()
  std::vector<CloneableTransform> transforms_;

  /// Map of std::vector pointer to transforms for that list
  std::unordered_map<const void*, ListTransforms> list_transforms_;

  /// Symbol transform registered with ReplaceAll()
  SymbolTransform symbol_transform_;
};

}  // namespace tint

#endif  // SRC_CLONE_CONTEXT_H_
