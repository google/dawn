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

#ifndef SRC_TINT_LANG_MSL_TYPE_COOPERATIVE_TENSOR_H_
#define SRC_TINT_LANG_MSL_TYPE_COOPERATIVE_TENSOR_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/clone_context.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::msl::type {

/// A cooperative_tensor type.
class CooperativeTensor : public Castable<CooperativeTensor, core::type::Type> {
  public:
    /// Constructor
    /// @param kind the kind of the matrix
    /// @param M the `M` dimension of the matrix operation (the number of rows in the result)
    /// @param N the `N` dimension of the matrix operation (the number of columns in the result)
    /// @param K the `K` dimension of the matrix operation (the common dimension of LHS/RHS)
    /// @param input_type the input type of the matrix operation
    /// @param result_type the result type of the matrix operation
    CooperativeTensor(core::SubgroupMatrixKind kind,
                      uint32_t M,
                      uint32_t N,
                      uint32_t K,
                      const Type* input_type,
                      const Type* output_type);

    /// Destructor
    ~CooperativeTensor() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the kind of the matrix
    core::SubgroupMatrixKind Kind() const { return kind_; }
    /// @returns the M dimension of the matrix operation
    uint32_t M() const { return M_; }
    /// @returns the N dimension of the matrix operation
    uint32_t N() const { return N_; }
    /// @returns the K dimension of the matrix operation
    uint32_t K() const { return K_; }
    /// @returns the input type of the matrix operation that this type is used with
    const core::type::Type* InputType() const { return input_type_; }
    /// @returns the result type of the matrix operation that this type is used with
    const core::type::Type* ResultType() const { return result_type_; }

    /// @returns the name for this type that closely resembles how it would be declared in WGSL.
    std::string FriendlyName() const override;

    /// @returns the name for this type in an identifier safe string.
    std::string IdentifierName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    CooperativeTensor* Clone(core::type::CloneContext& ctx) const override;

  private:
    const core::SubgroupMatrixKind kind_;
    const uint32_t M_;
    const uint32_t N_;
    const uint32_t K_;
    const core::type::Type* const input_type_;
    const core::type::Type* const result_type_;
};

}  // namespace tint::msl::type

#endif  // SRC_TINT_LANG_MSL_TYPE_COOPERATIVE_TENSOR_H_
