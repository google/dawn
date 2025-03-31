// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/type/image.h"

#include <sstream>

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::type::Image);

namespace tint::spirv::type {
namespace {

std::ostream& operator<<(std::ostream& out, const Image::Dim dim) {
    switch (dim) {
        case Image::Dim::k1D:
            out << "1d";
            break;
        case Image::Dim::k2D:
            out << "2d";
            break;
        case Image::Dim::k3D:
            out << "3d";
            break;
        case Image::Dim::kCube:
            out << "cube";
            break;
        case Image::Dim::kRect:
            out << "rect";
            break;
        case Image::Dim::kBuffer:
            out << "buffer";
            break;
        case Image::Dim::kSubpassData:
            out << "subpass_data";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Image::Depth depth) {
    switch (depth) {
        case Image::Depth::kNotDepth:
            out << "not_depth";
            break;
        case Image::Depth::kDepth:
            out << "depth";
            break;
        case Image::Depth::kUnknown:
            out << "depth_unknown";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Image::Arrayed arrayed) {
    switch (arrayed) {
        case Image::Arrayed::kNonArrayed:
            out << "non_arrayed";
            break;
        case Image::Arrayed::kArrayed:
            out << "arrayed";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Image::MultiSampled ms) {
    switch (ms) {
        case Image::MultiSampled::kSingleSampled:
            out << "single_sampled";
            break;
        case Image::MultiSampled::kMultiSampled:
            out << "multi_sampled";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Image::Sampled sampled) {
    switch (sampled) {
        case Image::Sampled::kKnownAtRuntime:
            out << "sampled_known_at_runtime";
            break;
        case Image::Sampled::kSamplingCompatible:
            out << "sampling_compatible";
            break;
        case Image::Sampled::kReadWriteOpCompatible:
            out << "rw_op_compatible";
            break;
    }
    return out;
}

}  // namespace

Image::Image(const core::type::Type* sampled_type,
             Dim dim,
             Depth depth,
             Arrayed arrayed,
             MultiSampled ms,
             Sampled sampled,
             core::TexelFormat fmt,
             core::Access access)
    : Base(static_cast<size_t>(Hash(tint::TypeCode::Of<Image>().bits,
                                    sampled_type,
                                    dim,
                                    depth,
                                    arrayed,
                                    ms,
                                    sampled,
                                    fmt,
                                    access)),
           core::type::Flags{}),
      sampled_type_(sampled_type),
      dim_(dim),
      depth_(depth),
      arrayed_(arrayed),
      ms_(ms),
      sampled_(sampled),
      fmt_(fmt),
      access_(access) {}

bool Image::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Image>()) {
        return o->sampled_type_ == sampled_type_ && o->dim_ == dim_ && o->depth_ == depth_ &&
               o->arrayed_ == arrayed_ && o->ms_ == ms_ && o->sampled_ == sampled_ &&
               o->fmt_ == fmt_ && o->access_ == access_;
    }
    return false;
}

std::string Image::FriendlyName() const {
    std::stringstream str;

    str << "spirv.image<" << sampled_type_->FriendlyName();
    str << ", " << dim_ << ", " << depth_ << ", " << arrayed_;
    str << ", " << ms_ << ", " << sampled_ << ", " << fmt_ << ", " << access_;
    str << ">";

    return str.str();
}

Image* Image::Clone(core::type::CloneContext& ctx) const {
    auto* sampled_type = sampled_type_->Clone(ctx);
    return ctx.dst.mgr->Get<Image>(sampled_type, dim_, depth_, arrayed_, ms_, sampled_, fmt_,
                                   access_);
}

}  // namespace tint::spirv::type
