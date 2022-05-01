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

#ifndef SRC_TINT_READER_WGSL_PARSER_IMPL_DETAIL_H_
#define SRC_TINT_READER_WGSL_PARSER_IMPL_DETAIL_H_

#include <memory>

namespace tint::reader::wgsl::detail {

/// OperatorArrow is a traits helper for ParserImpl::Expect<T>::operator->() and
/// ParserImpl::Maybe<T>::operator->() so that pointer types are automatically
/// dereferenced. This simplifies usage by allowing
///  `result.value->field`
/// to be written as:
///  `result->field`
/// As well as reducing the amount of code, using the operator->() asserts that
/// the Expect<T> or Maybe<T> is not in an error state before dereferencing.
template <typename T>
struct OperatorArrow {
    /// type resolves to the return type for the operator->()
    using type = T*;
    /// @param val the value held by `ParserImpl::Expect<T>` or
    /// `ParserImpl::Maybe<T>`.
    /// @return a pointer to `val`
    static inline T* ptr(T& val) { return &val; }
};

/// OperatorArrow template specialization for std::unique_ptr<>.
template <typename T>
struct OperatorArrow<std::unique_ptr<T>> {
    /// type resolves to the return type for the operator->()
    using type = T*;
    /// @param val the value held by `ParserImpl::Expect<T>` or
    /// `ParserImpl::Maybe<T>`.
    /// @return the raw pointer held by `val`.
    static inline T* ptr(std::unique_ptr<T>& val) { return val.get(); }
};

/// OperatorArrow template specialization for T*.
template <typename T>
struct OperatorArrow<T*> {
    /// type resolves to the return type for the operator->()
    using type = T*;
    /// @param val the value held by `ParserImpl::Expect<T>` or
    /// `ParserImpl::Maybe<T>`.
    /// @return `val`.
    static inline T* ptr(T* val) { return val; }
};

}  // namespace tint::reader::wgsl::detail

#endif  // SRC_TINT_READER_WGSL_PARSER_IMPL_DETAIL_H_
