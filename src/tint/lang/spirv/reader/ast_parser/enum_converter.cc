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

#include "src/tint/lang/spirv/reader/ast_parser/enum_converter.h"

#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::spirv::reader {

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

core::AddressSpace EnumConverter::ToAddressSpace(const spv::StorageClass sc) {
    switch (sc) {
        case spv::StorageClass::Input:
            return core::AddressSpace::kIn;
        case spv::StorageClass::Output:
            return core::AddressSpace::kOut;
        case spv::StorageClass::Uniform:
            return core::AddressSpace::kUniform;
        case spv::StorageClass::Workgroup:
            return core::AddressSpace::kWorkgroup;
        case spv::StorageClass::UniformConstant:
            return core::AddressSpace::kUndefined;
        case spv::StorageClass::StorageBuffer:
            return core::AddressSpace::kStorage;
        case spv::StorageClass::Private:
            return core::AddressSpace::kPrivate;
        case spv::StorageClass::Function:
            return core::AddressSpace::kFunction;
        default:
            break;
    }

    Fail() << "unknown SPIR-V storage class: " << uint32_t(sc);
    return core::AddressSpace::kUndefined;
}

core::BuiltinValue EnumConverter::ToBuiltin(spv::BuiltIn b) {
    switch (b) {
        case spv::BuiltIn::Position:
            return core::BuiltinValue::kPosition;
        case spv::BuiltIn::VertexIndex:
            return core::BuiltinValue::kVertexIndex;
        case spv::BuiltIn::InstanceIndex:
            return core::BuiltinValue::kInstanceIndex;
        case spv::BuiltIn::FrontFacing:
            return core::BuiltinValue::kFrontFacing;
        case spv::BuiltIn::FragCoord:
            return core::BuiltinValue::kPosition;
        case spv::BuiltIn::FragDepth:
            return core::BuiltinValue::kFragDepth;
        case spv::BuiltIn::LocalInvocationId:
            return core::BuiltinValue::kLocalInvocationId;
        case spv::BuiltIn::LocalInvocationIndex:
            return core::BuiltinValue::kLocalInvocationIndex;
        case spv::BuiltIn::GlobalInvocationId:
            return core::BuiltinValue::kGlobalInvocationId;
        case spv::BuiltIn::NumWorkgroups:
            return core::BuiltinValue::kNumWorkgroups;
        case spv::BuiltIn::WorkgroupId:
            return core::BuiltinValue::kWorkgroupId;
        case spv::BuiltIn::SampleId:
            return core::BuiltinValue::kSampleIndex;
        case spv::BuiltIn::SampleMask:
            return core::BuiltinValue::kSampleMask;
        default:
            break;
    }

    Fail() << "unknown SPIR-V builtin: " << uint32_t(b);
    return core::BuiltinValue::kUndefined;
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

core::TexelFormat EnumConverter::ToTexelFormat(spv::ImageFormat fmt) {
    switch (fmt) {
        case spv::ImageFormat::Unknown:
            return core::TexelFormat::kUndefined;

        // 8 bit channels
        case spv::ImageFormat::Rgba8:
            return core::TexelFormat::kRgba8Unorm;
        case spv::ImageFormat::Rgba8Snorm:
            return core::TexelFormat::kRgba8Snorm;
        case spv::ImageFormat::Rgba8ui:
            return core::TexelFormat::kRgba8Uint;
        case spv::ImageFormat::Rgba8i:
            return core::TexelFormat::kRgba8Sint;

        // 16 bit channels
        case spv::ImageFormat::Rgba16ui:
            return core::TexelFormat::kRgba16Uint;
        case spv::ImageFormat::Rgba16i:
            return core::TexelFormat::kRgba16Sint;
        case spv::ImageFormat::Rgba16f:
            return core::TexelFormat::kRgba16Float;

        // 32 bit channels
        case spv::ImageFormat::R32ui:
            return core::TexelFormat::kR32Uint;
        case spv::ImageFormat::R32i:
            return core::TexelFormat::kR32Sint;
        case spv::ImageFormat::R32f:
            return core::TexelFormat::kR32Float;
        case spv::ImageFormat::Rg32ui:
            return core::TexelFormat::kRg32Uint;
        case spv::ImageFormat::Rg32i:
            return core::TexelFormat::kRg32Sint;
        case spv::ImageFormat::Rg32f:
            return core::TexelFormat::kRg32Float;
        case spv::ImageFormat::Rgba32ui:
            return core::TexelFormat::kRgba32Uint;
        case spv::ImageFormat::Rgba32i:
            return core::TexelFormat::kRgba32Sint;
        case spv::ImageFormat::Rgba32f:
            return core::TexelFormat::kRgba32Float;
        default:
            break;
    }
    Fail() << "invalid image format: " << int(fmt);
    return core::TexelFormat::kUndefined;
}

}  // namespace tint::spirv::reader
