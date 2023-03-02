// Copyright 2020 The Tint Authors
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

#ifndef SRC_TINT_READER_SPIRV_FAIL_STREAM_H_
#define SRC_TINT_READER_SPIRV_FAIL_STREAM_H_

#include "src/tint/utils/string_stream.h"

namespace tint::reader::spirv {

/// A FailStream object accumulates values onto a given stream,
/// and can be used to record failure by writing the false value
/// to given a pointer-to-bool.
class FailStream {
  public:
    /// Creates a new fail stream
    /// @param status_ptr where we will write false to indicate failure. Assumed
    /// to be a valid pointer to bool.
    /// @param out output stream where a message should be written to explain
    /// the failure
    FailStream(bool* status_ptr, utils::StringStream* out) : status_ptr_(status_ptr), out_(out) {}
    /// Copy constructor
    /// @param other the fail stream to clone
    FailStream(const FailStream& other) = default;

    /// Converts to a boolean status. A true result indicates success,
    /// and a false result indicates failure.
    /// @returns the status
    operator bool() const { return *status_ptr_; }
    /// Returns the current status value.  This can be more readable
    /// the conversion operator.
    /// @returns the status
    bool status() const { return *status_ptr_; }

    /// Records failure.
    /// @returns a FailStream
    FailStream& Fail() {
        *status_ptr_ = false;
        return *this;
    }

    /// Appends the given value to the message output stream.
    /// @param val the value to write to the output stream.
    /// @returns this object
    template <typename T>
    FailStream& operator<<(const T& val) {
        *out_ << val;
        return *this;
    }

  private:
    bool* status_ptr_;
    utils::StringStream* out_;
};

}  // namespace tint::reader::spirv

#endif  // SRC_TINT_READER_SPIRV_FAIL_STREAM_H_
