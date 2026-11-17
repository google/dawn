// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_CONTAINERS_FUNCTION_REF_H_
#define SRC_TINT_UTILS_CONTAINERS_FUNCTION_REF_H_

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace tint {

template <typename F>
class FunctionRef;

namespace detail {

/// FunctionRefCanReturn is satisfied if a callable returning `RESULT` can be called by a
/// FunctionRef returning `R`. If `R` is a reference, `RESULT` must be a reference that binds to it
/// without creating a temporary, as the temporary would be destroyed before the FunctionRef returns
/// the reference.
template <typename R, typename RESULT>
concept FunctionRefCanReturn =
    !std::is_reference_v<R> ||
    (std::is_reference_v<RESULT> &&
     std::is_convertible_v<std::remove_reference_t<RESULT>*, std::remove_reference_t<R>*>);

/// FunctionRefCallable is satisfied if the callable `F` can be called with `ARGS`, and its result
/// can be returned by a FunctionRef returning `R`.
template <typename F, typename R, typename... ARGS>
concept FunctionRefCallable = std::is_invocable_r_v<R, F&, ARGS...> &&
                              FunctionRefCanReturn<R, std::invoke_result_t<F&, ARGS...>>;

}  // namespace detail

/// FunctionRef is a non-owning reference to a callable with the signature `R(ARGS...)`. It is a
/// lightweight alternative to std::function for parameters that are only called during the call
/// they are passed to: it is two pointers in size, never allocates, and is cheap to copy, so it
/// should be passed by value.
///
/// A FunctionRef references a callable object, such as a lambda. It does not extend the lifetime of
/// the object, so a FunctionRef must not outlive the object it was constructed from. In particular,
/// a FunctionRef constructed from a lambda expression must not be stored beyond the full expression
/// that created it.
///
/// Functions and function pointers are not accepted, as referencing them would need a cast between
/// function and object pointer types. To reference a function, wrap the call in a lambda.
///
/// Like std::function_ref (C++26), a FunctionRef always references a callable, and there is no
/// empty state.
template <typename R, typename... ARGS>
class FunctionRef<R(ARGS...)> {
  public:
    /// Constructor from a callable object, such as a lambda.
    /// @param obj the callable object to reference. Must outlive the FunctionRef.
    template <typename F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, FunctionRef> &&
                 !std::is_function_v<std::remove_reference_t<F>> &&
                 !std::is_pointer_v<std::remove_cvref_t<F>> &&
                 detail::FunctionRefCallable<F, R, ARGS...>)
    explicit(false) FunctionRef(F&& obj)
        // The const is restored by the cast in call_, so the object is never modified through a
        // non-const path if it was const.
        : obj_(const_cast<void*>(static_cast<const void*>(std::addressof(obj)))),
          call_([](void* o, ARGS... args) -> R {
              using T = std::remove_reference_t<F>;
              return Invoke(*static_cast<T*>(o), std::forward<ARGS>(args)...);
          }) {}

    /// Copy constructor
    FunctionRef(const FunctionRef&) = default;

    /// Copy assignment operator
    /// @returns this FunctionRef
    FunctionRef& operator=(const FunctionRef&) = default;

    /// Assignment from anything other than a FunctionRef is deleted, as a FunctionRef assigned a
    /// lambda expression would immediately reference a destroyed object.
    template <typename F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, FunctionRef>)
    FunctionRef& operator=(F&&) = delete;

    /// Calls the referenced callable.
    /// @param args the arguments to pass to the callable
    /// @returns the value returned by the callable, converted to `R`
    R operator()(ARGS... args) const { return call_(obj_, std::forward<ARGS>(args)...); }

  private:
    /// Calls @p callable with @p args, discarding the result if `R` is void.
    template <typename CALLABLE>
    static R Invoke(CALLABLE&& callable, ARGS&&... args) {
        if constexpr (std::is_void_v<R>) {
            std::invoke(std::forward<CALLABLE>(callable), std::forward<ARGS>(args)...);
        } else {
            return std::invoke(std::forward<CALLABLE>(callable), std::forward<ARGS>(args)...);
        }
    }

    /// The referenced callable object
    void* obj_;
    /// The function that casts #obj_ back to the callable's type, and calls it
    R (*call_)(void*, ARGS...);
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_FUNCTION_REF_H_
