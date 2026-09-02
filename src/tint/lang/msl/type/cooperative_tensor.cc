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

#include "src/tint/lang/msl/type/cooperative_tensor.h"

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::msl::type::CooperativeTensor);

namespace tint::msl::type {

CooperativeTensor::CooperativeTensor(core::SubgroupMatrixKind kind,
                                     uint32_t M,
                                     uint32_t N,
                                     uint32_t K,
                                     const core::type::Type* input_type,
                                     const core::type::Type* result_type)
    : Base(Hash(tint::TypeCode::Of<CooperativeTensor>().bits,
                kind,
                M,
                N,
                K,
                input_type,
                result_type),
           core::type::Flags{
               core::type::Flag::kConstructable,
               core::type::Flag::kCreationFixedFootprint,
               core::type::Flag::kFixedFootprint,
           }),
      kind_(kind),
      M_(M),
      N_(N),
      K_(K),
      input_type_(input_type),
      result_type_(result_type) {}

CooperativeTensor::~CooperativeTensor() = default;

bool CooperativeTensor::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<CooperativeTensor>()) {
        return v->kind_ == kind_ && v->M_ == M_ && v->N_ == N_ && v->K_ == K_ &&
               v->input_type_ == input_type_ && v->result_type_ == result_type_;
    }
    return false;
}

std::string CooperativeTensor::FriendlyName() const {
    StringStream out;
    out << "msl.cooperative_tensor_";
    switch (kind_) {
        case core::SubgroupMatrixKind::kLeft:
            out << "left";
            break;
        case core::SubgroupMatrixKind::kRight:
            out << "right";
            break;
        case core::SubgroupMatrixKind::kResult:
            out << "result";
            break;
        case core::SubgroupMatrixKind::kUndefined:
            TINT_UNREACHABLE();
    }
    out << "<" << M_ << ", " << N_ << ", " << K_ << ", " << input_type_->FriendlyName() << ", "
        << result_type_->FriendlyName() << ">";
    return out.str();
}

std::string CooperativeTensor::IdentifierName() const {
    StringStream out;
    out << "msl_cooperative_tensor_";
    switch (kind_) {
        case core::SubgroupMatrixKind::kLeft:
            out << "left";
            break;
        case core::SubgroupMatrixKind::kRight:
            out << "right";
            break;
        case core::SubgroupMatrixKind::kResult:
            out << "result";
            break;
        case core::SubgroupMatrixKind::kUndefined:
            TINT_UNREACHABLE();
    }
    out << "_" << M_ << "_" << N_ << "_" << K_ << "_" << input_type_->FriendlyName() << "_"
        << result_type_->FriendlyName();
    return out.str();
}

CooperativeTensor* CooperativeTensor::Clone(core::type::CloneContext& ctx) const {
    auto* input_ty = input_type_->Clone(ctx);
    auto* result_ty = result_type_->Clone(ctx);
    return ctx.dst.mgr->Get<CooperativeTensor>(kind_, M_, N_, K_, input_ty, result_ty);
}

}  // namespace tint::msl::type
