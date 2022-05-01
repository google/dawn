// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_MUTATOR_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_MUTATOR_H_

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace tint::fuzzers::spvtools_fuzzer {

/// This is an interface that is used to define custom mutators based on the
/// SPIR-V tools.
class Mutator {
  public:
    /// The status of the mutation.
    enum class Status {
        /// Binary is valid, the limit is not reached - can mutate further.
        kComplete,

        /// The binary is valid, the limit of mutations has been reached -
        /// can't mutate further.
        kLimitReached,

        /// The binary is valid, the limit is not reached but the mutator has spent
        /// too much time without mutating anything - better to restart to make sure
        /// we can make any progress.
        kStuck,

        /// The binary is invalid - this is likely a bug in the mutator - must
        /// abort.
        kInvalid
    };

    /// Represents the result of the mutation. The following states are possible:
    /// - if `IsChanged() == false`, then `GetStatus()` can be either
    ///   `kLimitReached` or `kStuck`.
    /// - otherwise, any value of `Status` is possible.
    class Result {
      public:
        /// Constructor.
        /// @param status - the status of the mutation.
        /// @param is_changed - whether the module was changed during mutation.
        Result(Status status, bool is_changed);

        /// @return the status of the mutation.
        Status GetStatus() const { return status_; }

        /// @return whether the module was changed during mutation.
        bool IsChanged() const { return is_changed_; }

      private:
        Status status_;
        bool is_changed_;
    };

    /// Virtual destructor.
    virtual ~Mutator();

    /// Causes the mutator to apply a mutation. This method can be called
    /// multiple times as long as the previous call didn't return
    /// `Status::kInvalid`.
    ///
    /// @return the status of the mutation (e.g. success, error etc) and whether
    ///     the binary was changed during mutation.
    virtual Result Mutate() = 0;

    /// Returns the mutated binary. The returned binary is guaranteed to be valid
    /// iff the previous call to the `Mutate` method returned didn't return
    /// `Status::kInvalid`.
    ///
    /// @return the mutated SPIR-V binary. It might be identical to the original
    ///     binary if `Result::IsChanged` returns `false`.
    virtual std::vector<uint32_t> GetBinary() const = 0;

    /// Returns errors, produced by the mutator.
    ///
    /// @param path - the directory to which the errors are printed to. No files
    ///     are created if the `path` is nullptr.
    /// @param count - the number of the error. Files for this error will be
    ///     prefixed with `count`.
    virtual void LogErrors(const std::string* path, uint32_t count) const = 0;

    /// @return errors encountered during the mutation. The returned string is
    ///     if there were no errors during mutation.
    virtual std::string GetErrors() const = 0;
};

}  // namespace tint::fuzzers::spvtools_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_MUTATOR_H_
