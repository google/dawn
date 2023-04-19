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

#ifndef SRC_TINT_CLONE_CONTEXT_H_
#define SRC_TINT_CLONE_CONTEXT_H_

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/tint/debug.h"
#include "src/tint/program_id.h"
#include "src/tint/symbol.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/hashset.h"
#include "src/tint/utils/traits.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint {
class CloneContext;
class Program;
class ProgramBuilder;
}  // namespace tint
namespace tint::ast {
class FunctionList;
class Node;
struct Type;
}  // namespace tint::ast

namespace tint {

ProgramID ProgramIDOf(const Program*);
ProgramID ProgramIDOf(const ProgramBuilder*);

/// Cloneable is the base class for all objects that can be cloned
class Cloneable : public utils::Castable<Cloneable> {
  public:
    /// Constructor
    Cloneable();
    /// Move constructor
    Cloneable(Cloneable&&);
    /// Destructor
    ~Cloneable() override;

    /// Performs a deep clone of this object using the CloneContext `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned object
    virtual const Cloneable* Clone(CloneContext* ctx) const = 0;
};

/// @returns an invalid ProgramID
inline ProgramID ProgramIDOf(const Cloneable*) {
    return ProgramID();
}

/// CloneContext holds the state used while cloning AST nodes.
class CloneContext {
    /// ParamTypeIsPtrOf<F, T> is true iff the first parameter of
    /// F is a pointer of (or derives from) type T.
    template <typename F, typename T>
    static constexpr bool ParamTypeIsPtrOf = utils::traits::
        IsTypeOrDerived<typename std::remove_pointer<utils::traits::ParameterType<F, 0>>::type, T>;

  public:
    /// SymbolTransform is a function that takes a symbol and returns a new
    /// symbol.
    using SymbolTransform = std::function<Symbol(Symbol)>;

    /// Constructor for cloning objects from `from` into `to`.
    /// @param to the target ProgramBuilder to clone into
    /// @param from the source Program to clone from
    /// @param auto_clone_symbols clone all symbols in `from` before returning
    CloneContext(ProgramBuilder* to, Program const* from, bool auto_clone_symbols = true);

    /// Constructor for cloning objects from and to the ProgramBuilder `builder`.
    /// @param builder the ProgramBuilder
    explicit CloneContext(ProgramBuilder* builder);

    /// Destructor
    ~CloneContext();

    /// Clones the Node or type::Type `a` into the ProgramBuilder #dst if `a` is
    /// not null. If `a` is null, then Clone() returns null.
    ///
    /// Clone() may use a function registered with ReplaceAll() to create a
    /// transformed version of the object. See ReplaceAll() for more information.
    ///
    /// If the CloneContext is cloning from a Program to a ProgramBuilder, then
    /// the Node or type::Type `a` must be owned by the Program #src.
    ///
    /// @param object the type deriving from Cloneable to clone
    /// @return the cloned node
    template <typename T>
    const T* Clone(const T* object) {
        if (src) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, object);
        }
        if (auto* cloned = CloneCloneable(object)) {
            auto* out = CheckedCast<T>(cloned);
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, dst, out);
            return out;
        }
        return nullptr;
    }

    /// Clones the Node or type::Type `a` into the ProgramBuilder #dst if `a` is
    /// not null. If `a` is null, then Clone() returns null.
    ///
    /// Unlike Clone(), this method does not invoke or use any transformations
    /// registered by ReplaceAll().
    ///
    /// If the CloneContext is cloning from a Program to a ProgramBuilder, then
    /// the Node or type::Type `a` must be owned by the Program #src.
    ///
    /// @param a the type deriving from Cloneable to clone
    /// @return the cloned node
    template <typename T>
    const T* CloneWithoutTransform(const T* a) {
        // If the input is nullptr, there's nothing to clone - just return nullptr.
        if (a == nullptr) {
            return nullptr;
        }
        if (src) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, a);
        }
        auto* c = a->Clone(this);
        return CheckedCast<T>(c);
    }

    /// Clones the ast::Type `ty` into the ProgramBuilder #dst
    /// @param ty the AST type.
    /// @return the cloned node
    ast::Type Clone(const ast::Type& ty);

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
    template <typename T, size_t N>
    utils::Vector<T, N> Clone(const utils::Vector<T, N>& v) {
        utils::Vector<T, N> out;
        out.reserve(v.size());
        for (auto& el : v) {
            out.Push(Clone(el));
        }
        return out;
    }

    /// Clones each of the elements of the vector `v` using the ProgramBuilder
    /// #dst, inserting any additional elements into the list that were registered
    /// with calls to InsertBefore().
    ///
    /// All the elements of the vector `v` must be owned by the Program #src.
    ///
    /// @param v the vector to clone
    /// @return the cloned vector
    template <typename T, size_t N>
    utils::Vector<T*, N> Clone(const utils::Vector<T*, N>& v) {
        utils::Vector<T*, N> out;
        Clone(out, v);
        return out;
    }

    /// Clones each of the elements of the vector `from` into the vector `to`,
    /// inserting any additional elements into the list that were registered with
    /// calls to InsertBefore().
    ///
    /// All the elements of the vector `from` must be owned by the Program #src.
    ///
    /// @param from the vector to clone
    /// @param to the cloned result
    template <typename T, size_t N>
    void Clone(utils::Vector<T*, N>& to, const utils::Vector<T*, N>& from) {
        to.Reserve(from.Length());

        auto transforms = list_transforms_.Find(&from);

        if (transforms) {
            for (auto& builder : transforms->insert_front_) {
                to.Push(CheckedCast<T>(builder()));
            }
            for (auto& el : from) {
                if (auto insert_before = transforms->insert_before_.Find(el)) {
                    for (auto& builder : *insert_before) {
                        to.Push(CheckedCast<T>(builder()));
                    }
                }
                if (!transforms->remove_.Contains(el)) {
                    to.Push(Clone(el));
                }
                if (auto insert_after = transforms->insert_after_.Find(el)) {
                    for (auto& builder : *insert_after) {
                        to.Push(CheckedCast<T>(builder()));
                    }
                }
            }
            for (auto& builder : transforms->insert_back_) {
                to.Push(CheckedCast<T>(builder()));
            }
        } else {
            for (auto& el : from) {
                to.Push(Clone(el));

                // Clone(el) may have updated the transformation list, adding an `insert_after`
                // transform for `from`.
                if (transforms) {
                    if (auto insert_after = transforms->insert_after_.Find(el)) {
                        for (auto& builder : *insert_after) {
                            to.Push(CheckedCast<T>(builder()));
                        }
                    }
                }
            }

            // Clone(el) may have updated the transformation list, adding an `insert_back_`
            // transform for `from`.
            if (transforms) {
                for (auto& builder : transforms->insert_back_) {
                    to.Push(CheckedCast<T>(builder()));
                }
            }
        }
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
    ///   // Replace all ast::UintLiteralExpressions with the number 42
    ///   CloneCtx ctx(&out, in);
    ///   ctx.ReplaceAll([&] (ast::UintLiteralExpression* l) {
    ///       return ctx->dst->create<ast::UintLiteralExpression>(
    ///           ctx->Clone(l->source),
    ///           ctx->Clone(l->type),
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
    utils::traits::EnableIf<ParamTypeIsPtrOf<F, Cloneable>, CloneContext>& ReplaceAll(
        F&& replacer) {
        using TPtr = utils::traits::ParameterType<F, 0>;
        using T = typename std::remove_pointer<TPtr>::type;
        for (auto& transform : transforms_) {
            bool already_registered = transform.typeinfo->Is(&utils::TypeInfo::Of<T>()) ||
                                      utils::TypeInfo::Of<T>().Is(transform.typeinfo);
            if (TINT_UNLIKELY(already_registered)) {
                TINT_ICE(Clone, Diagnostics()) << "ReplaceAll() called with a handler for type "
                                               << utils::TypeInfo::Of<T>().name
                                               << " that is already handled by a handler for type "
                                               << transform.typeinfo->name;
                return *this;
            }
        }
        CloneableTransform transform;
        transform.typeinfo = &utils::TypeInfo::Of<T>();
        transform.function = [=](const Cloneable* in) { return replacer(in->As<T>()); };
        transforms_.Push(std::move(transform));
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
        if (TINT_UNLIKELY(symbol_transform_)) {
            TINT_ICE(Clone, Diagnostics()) << "ReplaceAll(const SymbolTransform&) called "
                                              "multiple times on the same CloneContext";
            return *this;
        }
        symbol_transform_ = replacer;
        return *this;
    }

    /// Replace replaces all occurrences of `what` in #src with the pointer `with`
    /// in #dst when calling Clone().
    /// [DEPRECATED]: This function cannot handle nested replacements. Use the
    /// overload of Replace() that take a function for the `WITH` argument.
    /// @param what a pointer to the object in #src that will be replaced with
    /// `with`
    /// @param with a pointer to the replacement object owned by #dst that will be
    /// used as a replacement for `what`
    /// @warning The replacement object must be of the correct type for all
    /// references of the original object. A type mismatch will result in an
    /// assertion in debug builds, and undefined behavior in release builds.
    /// @returns this CloneContext so calls can be chained
    template <typename WHAT,
              typename WITH,
              typename = utils::traits::EnableIfIsType<WITH, Cloneable>>
    CloneContext& Replace(const WHAT* what, const WITH* with) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, what);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, dst, with);
        replacements_.Replace(what, [with]() -> const Cloneable* { return with; });
        return *this;
    }

    /// Replace replaces all occurrences of `what` in #src with the result of the
    /// function `with` in #dst when calling Clone(). `with` will be called each
    /// time `what` is cloned by this context. If `what` is not cloned, then
    /// `with` may never be called.
    /// @param what a pointer to the object in #src that will be replaced with
    /// `with`
    /// @param with a function that takes no arguments and returns a pointer to
    /// the replacement object owned by #dst. The returned pointer will be used as
    /// a replacement for `what`.
    /// @warning The replacement object must be of the correct type for all
    /// references of the original object. A type mismatch will result in an
    /// assertion in debug builds, and undefined behavior in release builds.
    /// @returns this CloneContext so calls can be chained
    template <typename WHAT, typename WITH, typename = std::invoke_result_t<WITH>>
    CloneContext& Replace(const WHAT* what, WITH&& with) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, what);
        replacements_.Replace(what, with);
        return *this;
    }

    /// Removes @p object from the cloned copy of @p vector.
    /// @param vector the vector in #src
    /// @param object a pointer to the object in #src that will be omitted from
    /// the cloned vector.
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename OBJECT>
    CloneContext& Remove(const utils::Vector<T, N>& vector, OBJECT* object) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, object);
        if (TINT_UNLIKELY((std::find(vector.begin(), vector.end(), object) == vector.end()))) {
            TINT_ICE(Clone, Diagnostics())
                << "CloneContext::Remove() vector does not contain object";
            return *this;
        }

        list_transforms_.GetOrZero(&vector)->remove_.Add(object);
        return *this;
    }

    /// Inserts @p object before any other objects of @p vector, when the vector is cloned.
    /// @param vector the vector in #src
    /// @param object a pointer to the object in #dst that will be inserted at the
    /// front of the vector
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename OBJECT>
    CloneContext& InsertFront(const utils::Vector<T, N>& vector, OBJECT* object) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, dst, object);
        return InsertFront(vector, [object] { return object; });
    }

    /// Inserts a lazily built object before any other objects of @p vector, when the vector is
    /// cloned.
    /// @param vector the vector in #src
    /// @param builder a builder of the object that will be inserted at the front of the vector.
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename BUILDER>
    CloneContext& InsertFront(const utils::Vector<T, N>& vector, BUILDER&& builder) {
        list_transforms_.GetOrZero(&vector)->insert_front_.Push(std::forward<BUILDER>(builder));
        return *this;
    }

    /// Inserts @p object after any other objects of @p vector, when the vector is cloned.
    /// @param vector the vector in #src
    /// @param object a pointer to the object in #dst that will be inserted at the
    /// end of the vector
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename OBJECT>
    CloneContext& InsertBack(const utils::Vector<T, N>& vector, OBJECT* object) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, dst, object);
        return InsertBack(vector, [object] { return object; });
    }

    /// Inserts a lazily built object after any other objects of @p vector, when the vector is
    /// cloned.
    /// @param vector the vector in #src
    /// @param builder the builder of the object in #dst that will be inserted at the end of the
    /// vector.
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename BUILDER>
    CloneContext& InsertBack(const utils::Vector<T, N>& vector, BUILDER&& builder) {
        list_transforms_.GetOrZero(&vector)->insert_back_.Push(std::forward<BUILDER>(builder));
        return *this;
    }

    /// Inserts @p object before @p before whenever @p vector is cloned.
    /// @param vector the vector in #src
    /// @param before a pointer to the object in #src
    /// @param object a pointer to the object in #dst that will be inserted before
    /// any occurrence of the clone of @p before
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename BEFORE, typename OBJECT>
    CloneContext& InsertBefore(const utils::Vector<T, N>& vector,
                               const BEFORE* before,
                               const OBJECT* object) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, before);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, dst, object);
        if (TINT_UNLIKELY((std::find(vector.begin(), vector.end(), before) == vector.end()))) {
            TINT_ICE(Clone, Diagnostics())
                << "CloneContext::InsertBefore() vector does not contain before";
            return *this;
        }

        list_transforms_.GetOrZero(&vector)->insert_before_.GetOrZero(before)->Push(
            [object] { return object; });
        return *this;
    }

    /// Inserts a lazily created object before @p before whenever @p vector is cloned.
    /// @param vector the vector in #src
    /// @param before a pointer to the object in #src
    /// @param builder the builder of the object in #dst that will be inserted before any occurrence
    /// of the clone of @p before
    /// @returns this CloneContext so calls can be chained
    template <typename T,
              size_t N,
              typename BEFORE,
              typename BUILDER,
              typename _ = std::enable_if_t<!std::is_pointer_v<std::decay_t<BUILDER>>>>
    CloneContext& InsertBefore(const utils::Vector<T, N>& vector,
                               const BEFORE* before,
                               BUILDER&& builder) {
        list_transforms_.GetOrZero(&vector)->insert_before_.GetOrZero(before)->Push(
            std::forward<BUILDER>(builder));
        return *this;
    }

    /// Inserts @p object after @p after whenever @p vector is cloned.
    /// @param vector the vector in #src
    /// @param after a pointer to the object in #src
    /// @param object a pointer to the object in #dst that will be inserted after
    /// any occurrence of the clone of @p after
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename AFTER, typename OBJECT>
    CloneContext& InsertAfter(const utils::Vector<T, N>& vector,
                              const AFTER* after,
                              const OBJECT* object) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, src, after);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Clone, dst, object);
        if (TINT_UNLIKELY((std::find(vector.begin(), vector.end(), after) == vector.end()))) {
            TINT_ICE(Clone, Diagnostics())
                << "CloneContext::InsertAfter() vector does not contain after";
            return *this;
        }

        list_transforms_.GetOrZero(&vector)->insert_after_.GetOrZero(after)->Push(
            [object] { return object; });
        return *this;
    }

    /// Inserts a lazily created object after @p after whenever @p vector is cloned.
    /// @param vector the vector in #src
    /// @param after a pointer to the object in #src
    /// @param builder the builder of the object in #dst that will be inserted after any occurrence
    /// of the clone of @p after
    /// @returns this CloneContext so calls can be chained
    template <typename T,
              size_t N,
              typename AFTER,
              typename BUILDER,
              typename _ = std::enable_if_t<!std::is_pointer_v<std::decay_t<BUILDER>>>>
    CloneContext& InsertAfter(const utils::Vector<T, N>& vector,
                              const AFTER* after,
                              BUILDER&& builder) {
        list_transforms_.GetOrZero(&vector)->insert_after_.GetOrZero(after)->Push(
            std::forward<BUILDER>(builder));
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

        // utils::TypeInfo of the Cloneable that the transform operates on
        const utils::TypeInfo* typeinfo;
        std::function<const Cloneable*(const Cloneable*)> function;
    };

    /// A vector of const Cloneable*
    using CloneableBuilderList = utils::Vector<std::function<const Cloneable*()>, 4>;

    /// Transformations to be applied to a list (vector)
    struct ListTransforms {
        /// A map of object in #src to omit when cloned into #dst.
        utils::Hashset<const Cloneable*, 4> remove_;

        /// A list of objects in #dst to insert before any others when the vector is cloned.
        CloneableBuilderList insert_front_;

        /// A list of objects in #dst to insert after all others when the vector is cloned.
        CloneableBuilderList insert_back_;

        /// A map of object in #src to the list of cloned objects in #dst.
        /// Clone(const utils::Vector<T*>& v) will use this to insert the map-value
        /// list into the target vector before cloning and inserting the map-key.
        utils::Hashmap<const Cloneable*, CloneableBuilderList, 4> insert_before_;

        /// A map of object in #src to the list of cloned objects in #dst.
        /// Clone(const utils::Vector<T*>& v) will use this to insert the map-value
        /// list into the target vector after cloning and inserting the map-key.
        utils::Hashmap<const Cloneable*, CloneableBuilderList, 4> insert_after_;
    };

    CloneContext(const CloneContext&) = delete;
    CloneContext& operator=(const CloneContext&) = delete;

    /// Cast `obj` from type `FROM` to type `TO`, returning the cast object.
    /// Reports an internal compiler error if the cast failed.
    template <typename TO, typename FROM>
    const TO* CheckedCast(const FROM* obj) {
        if (obj == nullptr) {
            return nullptr;
        }
        const TO* cast = obj->template As<TO>();
        if (TINT_LIKELY(cast)) {
            return cast;
        }
        CheckedCastFailure(obj, utils::TypeInfo::Of<TO>());
        return nullptr;
    }

    /// Clones a Cloneable object, using any replacements or transforms that have
    /// been configured.
    const Cloneable* CloneCloneable(const Cloneable* object);

    /// Adds an error diagnostic to Diagnostics() that the cloned object was not
    /// of the expected type.
    void CheckedCastFailure(const Cloneable* got, const utils::TypeInfo& expected);

    /// @returns the diagnostic list of #dst
    diag::List& Diagnostics() const;

    /// A map of object in #src to functions that create their replacement in #dst
    utils::Hashmap<const Cloneable*, std::function<const Cloneable*()>, 8> replacements_;

    /// A map of symbol in #src to their cloned equivalent in #dst
    utils::Hashmap<Symbol, Symbol, 32> cloned_symbols_;

    /// Cloneable transform functions registered with ReplaceAll()
    utils::Vector<CloneableTransform, 8> transforms_;

    /// Transformations to apply to vectors
    utils::Hashmap<const void*, ListTransforms, 4> list_transforms_;

    /// Symbol transform registered with ReplaceAll()
    SymbolTransform symbol_transform_;
};

}  // namespace tint

#endif  // SRC_TINT_CLONE_CONTEXT_H_
