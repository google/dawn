// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_RESULT_H_
#define SRC_TINT_UTILS_RESULT_H_

#include <utility>
#include <variant>

#include "src/tint/debug.h"
#include "src/tint/utils/string_stream.h"

namespace tint::utils {

/// Empty structure used as the default FAILURE_TYPE for a Result.
struct FailureType {};

static constexpr const FailureType Failure;

/// Result is a helper for functions that need to return a value, or an failure value.
/// Result can be constructed with either a 'success' or 'failure' value.
/// @tparam SUCCESS_TYPE the 'success' value type.
/// @tparam FAILURE_TYPE the 'failure' value type. Defaults to FailureType which provides no
///         information about the failure, except that something failed. Must not be the same type
///         as SUCCESS_TYPE.
template <typename SUCCESS_TYPE, typename FAILURE_TYPE = FailureType>
struct [[nodiscard]] Result {
    static_assert(!std::is_same_v<SUCCESS_TYPE, FAILURE_TYPE>,
                  "Result must not have the same type for SUCCESS_TYPE and FAILURE_TYPE");

    /// Default constructor initializes to invalid state
    Result() : value(std::monostate{}) {}

    /// Constructor
    /// @param success the success result
    Result(const SUCCESS_TYPE& success)  // NOLINT(runtime/explicit):
        : value{success} {}

    /// Constructor
    /// @param success the success result
    Result(SUCCESS_TYPE&& success)  // NOLINT(runtime/explicit):
        : value(std::move(SUCCESS_TYPE(std::move(success)))) {}

    /// Constructor
    /// @param failure the failure result
    Result(const FAILURE_TYPE& failure)  // NOLINT(runtime/explicit):
        : value{failure} {}

    /// Constructor
    /// @param failure the failure result
    Result(FAILURE_TYPE&& failure)  // NOLINT(runtime/explicit):
        : value{std::move(failure)} {}

    /// Copy constructor with success / failure casting
    /// @param other the Result to copy
    template <typename S,
              typename F,
              typename = std::void_t<decltype(SUCCESS_TYPE{std::declval<S>()}),
                                     decltype(FAILURE_TYPE{std::declval<F>()})>>
    Result(const Result<S, F>& other) {  // NOLINT(runtime/explicit):
        if (other) {
            value = SUCCESS_TYPE{other.Get()};
        } else {
            value = FAILURE_TYPE{other.Failure()};
        }
    }

    /// @returns true if the result was a success
    operator bool() const {
        Validate();
        return std::holds_alternative<SUCCESS_TYPE>(value);
    }

    /// @returns true if the result was a failure
    bool operator!() const {
        Validate();
        return std::holds_alternative<FAILURE_TYPE>(value);
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure will result in UB.
    const SUCCESS_TYPE* operator->() const {
        Validate();
        return &(Get());
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure value will result in UB.
    const SUCCESS_TYPE& Get() const {
        Validate();
        return std::get<SUCCESS_TYPE>(value);
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure value will result in UB.
    SUCCESS_TYPE&& Move() {
        Validate();
        return std::get<SUCCESS_TYPE>(std::move(value));
    }

    /// @returns the failure value
    /// @warning attempting to call this when the Result holds a success value will result in UB.
    const FAILURE_TYPE& Failure() const {
        Validate();
        return std::get<FAILURE_TYPE>(value);
    }

    /// Equality operator
    /// @param val the value to compare this Result to
    /// @returns true if this result holds a success value equal to `value`
    bool operator==(SUCCESS_TYPE val) const {
        Validate();
        if (auto* v = std::get_if<SUCCESS_TYPE>(&value)) {
            return *v == val;
        }
        return false;
    }

    /// Equality operator
    /// @param val the value to compare this Result to
    /// @returns true if this result holds a failure value equal to `value`
    bool operator==(FAILURE_TYPE val) const {
        Validate();
        if (auto* v = std::get_if<FAILURE_TYPE>(&value)) {
            return *v == val;
        }
        return false;
    }

  private:
    void Validate() const { TINT_ASSERT(Utils, !std::holds_alternative<std::monostate>(value)); }

    /// The result. Either a success of failure value.
    std::variant<std::monostate, SUCCESS_TYPE, FAILURE_TYPE> value;
};

/// Writes the result to the stream.
/// @param out the stream to write to
/// @param res the result
/// @return the stream so calls can be chained
template <typename SUCCESS, typename FAILURE>
inline utils::StringStream& operator<<(utils::StringStream& out, Result<SUCCESS, FAILURE> res) {
    return res ? (out << "success: " << res.Get()) : (out << "failure: " << res.Failure());
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_RESULT_H_
