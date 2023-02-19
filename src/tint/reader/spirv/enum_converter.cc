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

#include "src/tint/reader/spirv/enum_converter.h"

#include "src/tint/type/texture_dimension.h"

namespace tint::reader::spirv {

EnumConverter::EnumConverter(const FailStream& fs) : fail_stream_(fs) {}

EnumConverter::~EnumConverter() = default;

ast::PipelineStage EnumConverter::ToPipelineStage(spv::ExecutionModel model) {
    switch (model) {
        case spv::ExecutionModel::Vertex:
            return ast::PipelineStage::kVertex;
        case spv::ExecutionModel::Fragment:
            return ast::PipelineStage::kFragment;
        case spv::ExecutionModel::GLCompute:
            return ast::PipelineStage::kCompute;
        default:
            break;
    }

    Fail() << "unknown SPIR-V execution model: " << uint32_t(model);
    return ast::PipelineStage::kNone;
}

builtin::AddressSpace EnumConverter::ToAddressSpace(const spv::StorageClass sc) {
    switch (sc) {
        case spv::StorageClass::Input:
            return builtin::AddressSpace::kIn;
        case spv::StorageClass::Output:
            return builtin::AddressSpace::kOut;
        case spv::StorageClass::Uniform:
            return builtin::AddressSpace::kUniform;
        case spv::StorageClass::Workgroup:
            return builtin::AddressSpace::kWorkgroup;
        case spv::StorageClass::UniformConstant:
            return builtin::AddressSpace::kUndefined;
        case spv::StorageClass::StorageBuffer:
            return builtin::AddressSpace::kStorage;
        case spv::StorageClass::Private:
            return builtin::AddressSpace::kPrivate;
        case spv::StorageClass::Function:
            return builtin::AddressSpace::kFunction;
        default:
            break;
    }

    Fail() << "unknown SPIR-V storage class: " << uint32_t(sc);
    return builtin::AddressSpace::kUndefined;
}

builtin::BuiltinValue EnumConverter::ToBuiltin(spv::BuiltIn b) {
    switch (b) {
        case spv::BuiltIn::Position:
            return builtin::BuiltinValue::kPosition;
        case spv::BuiltIn::VertexIndex:
            return builtin::BuiltinValue::kVertexIndex;
        case spv::BuiltIn::InstanceIndex:
            return builtin::BuiltinValue::kInstanceIndex;
        case spv::BuiltIn::FrontFacing:
            return builtin::BuiltinValue::kFrontFacing;
        case spv::BuiltIn::FragCoord:
            return builtin::BuiltinValue::kPosition;
        case spv::BuiltIn::FragDepth:
            return builtin::BuiltinValue::kFragDepth;
        case spv::BuiltIn::LocalInvocationId:
            return builtin::BuiltinValue::kLocalInvocationId;
        case spv::BuiltIn::LocalInvocationIndex:
            return builtin::BuiltinValue::kLocalInvocationIndex;
        case spv::BuiltIn::GlobalInvocationId:
            return builtin::BuiltinValue::kGlobalInvocationId;
        case spv::BuiltIn::NumWorkgroups:
            return builtin::BuiltinValue::kNumWorkgroups;
        case spv::BuiltIn::WorkgroupId:
            return builtin::BuiltinValue::kWorkgroupId;
        case spv::BuiltIn::SampleId:
            return builtin::BuiltinValue::kSampleIndex;
        case spv::BuiltIn::SampleMask:
            return builtin::BuiltinValue::kSampleMask;
        default:
            break;
    }

    Fail() << "unknown SPIR-V builtin: " << uint32_t(b);
    return builtin::BuiltinValue::kUndefined;
}

type::TextureDimension EnumConverter::ToDim(spv::Dim dim, bool arrayed) {
    if (arrayed) {
        switch (dim) {
            case spv::Dim::Dim2D:
                return type::TextureDimension::k2dArray;
            case spv::Dim::Cube:
                return type::TextureDimension::kCubeArray;
            default:
                break;
        }
        Fail() << "arrayed dimension must be 2D or Cube. Got " << int(dim);
        return type::TextureDimension::kNone;
    }
    // Assume non-arrayed
    switch (dim) {
        case spv::Dim::Dim1D:
            return type::TextureDimension::k1d;
        case spv::Dim::Dim2D:
            return type::TextureDimension::k2d;
        case spv::Dim::Dim3D:
            return type::TextureDimension::k3d;
        case spv::Dim::Cube:
            return type::TextureDimension::kCube;
        default:
            break;
    }
    Fail() << "invalid dimension: " << int(dim);
    return type::TextureDimension::kNone;
}

builtin::TexelFormat EnumConverter::ToTexelFormat(spv::ImageFormat fmt) {
    switch (fmt) {
        case spv::ImageFormat::Unknown:
            return builtin::TexelFormat::kUndefined;

        // 8 bit channels
        case spv::ImageFormat::Rgba8:
            return builtin::TexelFormat::kRgba8Unorm;
        case spv::ImageFormat::Rgba8Snorm:
            return builtin::TexelFormat::kRgba8Snorm;
        case spv::ImageFormat::Rgba8ui:
            return builtin::TexelFormat::kRgba8Uint;
        case spv::ImageFormat::Rgba8i:
            return builtin::TexelFormat::kRgba8Sint;

        // 16 bit channels
        case spv::ImageFormat::Rgba16ui:
            return builtin::TexelFormat::kRgba16Uint;
        case spv::ImageFormat::Rgba16i:
            return builtin::TexelFormat::kRgba16Sint;
        case spv::ImageFormat::Rgba16f:
            return builtin::TexelFormat::kRgba16Float;

        // 32 bit channels
        case spv::ImageFormat::R32ui:
            return builtin::TexelFormat::kR32Uint;
        case spv::ImageFormat::R32i:
            return builtin::TexelFormat::kR32Sint;
        case spv::ImageFormat::R32f:
            return builtin::TexelFormat::kR32Float;
        case spv::ImageFormat::Rg32ui:
            return builtin::TexelFormat::kRg32Uint;
        case spv::ImageFormat::Rg32i:
            return builtin::TexelFormat::kRg32Sint;
        case spv::ImageFormat::Rg32f:
            return builtin::TexelFormat::kRg32Float;
        case spv::ImageFormat::Rgba32ui:
            return builtin::TexelFormat::kRgba32Uint;
        case spv::ImageFormat::Rgba32i:
            return builtin::TexelFormat::kRgba32Sint;
        case spv::ImageFormat::Rgba32f:
            return builtin::TexelFormat::kRgba32Float;
        default:
            break;
    }
    Fail() << "invalid image format: " << int(fmt);
    return builtin::TexelFormat::kUndefined;
}

}  // namespace tint::reader::spirv
