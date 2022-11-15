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

ast::AddressSpace EnumConverter::ToAddressSpace(const spv::StorageClass sc) {
    switch (sc) {
        case spv::StorageClass::Input:
            return ast::AddressSpace::kIn;
        case spv::StorageClass::Output:
            return ast::AddressSpace::kOut;
        case spv::StorageClass::Uniform:
            return ast::AddressSpace::kUniform;
        case spv::StorageClass::Workgroup:
            return ast::AddressSpace::kWorkgroup;
        case spv::StorageClass::UniformConstant:
            return ast::AddressSpace::kNone;
        case spv::StorageClass::StorageBuffer:
            return ast::AddressSpace::kStorage;
        case spv::StorageClass::Private:
            return ast::AddressSpace::kPrivate;
        case spv::StorageClass::Function:
            return ast::AddressSpace::kFunction;
        default:
            break;
    }

    Fail() << "unknown SPIR-V storage class: " << uint32_t(sc);
    return ast::AddressSpace::kUndefined;
}

ast::BuiltinValue EnumConverter::ToBuiltin(spv::BuiltIn b) {
    switch (b) {
        case spv::BuiltIn::Position:
            return ast::BuiltinValue::kPosition;
        case spv::BuiltIn::VertexIndex:
            return ast::BuiltinValue::kVertexIndex;
        case spv::BuiltIn::InstanceIndex:
            return ast::BuiltinValue::kInstanceIndex;
        case spv::BuiltIn::FrontFacing:
            return ast::BuiltinValue::kFrontFacing;
        case spv::BuiltIn::FragCoord:
            return ast::BuiltinValue::kPosition;
        case spv::BuiltIn::FragDepth:
            return ast::BuiltinValue::kFragDepth;
        case spv::BuiltIn::LocalInvocationId:
            return ast::BuiltinValue::kLocalInvocationId;
        case spv::BuiltIn::LocalInvocationIndex:
            return ast::BuiltinValue::kLocalInvocationIndex;
        case spv::BuiltIn::GlobalInvocationId:
            return ast::BuiltinValue::kGlobalInvocationId;
        case spv::BuiltIn::NumWorkgroups:
            return ast::BuiltinValue::kNumWorkgroups;
        case spv::BuiltIn::WorkgroupId:
            return ast::BuiltinValue::kWorkgroupId;
        case spv::BuiltIn::SampleId:
            return ast::BuiltinValue::kSampleIndex;
        case spv::BuiltIn::SampleMask:
            return ast::BuiltinValue::kSampleMask;
        default:
            break;
    }

    Fail() << "unknown SPIR-V builtin: " << uint32_t(b);
    return ast::BuiltinValue::kUndefined;
}

ast::TextureDimension EnumConverter::ToDim(spv::Dim dim, bool arrayed) {
    if (arrayed) {
        switch (dim) {
            case spv::Dim::Dim2D:
                return ast::TextureDimension::k2dArray;
            case spv::Dim::Cube:
                return ast::TextureDimension::kCubeArray;
            default:
                break;
        }
        Fail() << "arrayed dimension must be 2D or Cube. Got " << int(dim);
        return ast::TextureDimension::kNone;
    }
    // Assume non-arrayed
    switch (dim) {
        case spv::Dim::Dim1D:
            return ast::TextureDimension::k1d;
        case spv::Dim::Dim2D:
            return ast::TextureDimension::k2d;
        case spv::Dim::Dim3D:
            return ast::TextureDimension::k3d;
        case spv::Dim::Cube:
            return ast::TextureDimension::kCube;
        default:
            break;
    }
    Fail() << "invalid dimension: " << int(dim);
    return ast::TextureDimension::kNone;
}

ast::TexelFormat EnumConverter::ToTexelFormat(spv::ImageFormat fmt) {
    switch (fmt) {
        case spv::ImageFormat::Unknown:
            return ast::TexelFormat::kUndefined;

        // 8 bit channels
        case spv::ImageFormat::Rgba8:
            return ast::TexelFormat::kRgba8Unorm;
        case spv::ImageFormat::Rgba8Snorm:
            return ast::TexelFormat::kRgba8Snorm;
        case spv::ImageFormat::Rgba8ui:
            return ast::TexelFormat::kRgba8Uint;
        case spv::ImageFormat::Rgba8i:
            return ast::TexelFormat::kRgba8Sint;

        // 16 bit channels
        case spv::ImageFormat::Rgba16ui:
            return ast::TexelFormat::kRgba16Uint;
        case spv::ImageFormat::Rgba16i:
            return ast::TexelFormat::kRgba16Sint;
        case spv::ImageFormat::Rgba16f:
            return ast::TexelFormat::kRgba16Float;

        // 32 bit channels
        case spv::ImageFormat::R32ui:
            return ast::TexelFormat::kR32Uint;
        case spv::ImageFormat::R32i:
            return ast::TexelFormat::kR32Sint;
        case spv::ImageFormat::R32f:
            return ast::TexelFormat::kR32Float;
        case spv::ImageFormat::Rg32ui:
            return ast::TexelFormat::kRg32Uint;
        case spv::ImageFormat::Rg32i:
            return ast::TexelFormat::kRg32Sint;
        case spv::ImageFormat::Rg32f:
            return ast::TexelFormat::kRg32Float;
        case spv::ImageFormat::Rgba32ui:
            return ast::TexelFormat::kRgba32Uint;
        case spv::ImageFormat::Rgba32i:
            return ast::TexelFormat::kRgba32Sint;
        case spv::ImageFormat::Rgba32f:
            return ast::TexelFormat::kRgba32Float;
        default:
            break;
    }
    Fail() << "invalid image format: " << int(fmt);
    return ast::TexelFormat::kUndefined;
}

}  // namespace tint::reader::spirv
