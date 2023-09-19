// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/spirv/type/sampled_image.h"

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::type::SampledImage);

namespace tint::spirv::type {

SampledImage::SampledImage(const core::type::Type* image)
    : Base(static_cast<size_t>(Hash(tint::TypeInfo::Of<SampledImage>().full_hashcode, image)),
           core::type::Flags{}),
      image_(image) {}

SampledImage* SampledImage::Clone(core::type::CloneContext& ctx) const {
    auto* image = image_->Clone(ctx);
    return ctx.dst.mgr->Get<SampledImage>(image);
}

}  // namespace tint::spirv::type
