// Copyright 2021 The Dawn Authors
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

#include "dawn/native/TintUtils.h"

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"

#include "tint/tint.h"

namespace dawn::native {

namespace {

thread_local DeviceBase* tlDevice = nullptr;

void TintICEReporter(const tint::InternalCompilerError& err) {
    if (tlDevice) {
        tlDevice->HandleError(DAWN_INTERNAL_ERROR(err.Error()));
#if DAWN_ENABLE_ASSERTS
        HandleAssertionFailure(err.File(), "", err.Line(), err.Message().c_str());
#endif
    }
}

bool InitializeTintErrorReporter() {
    tint::SetInternalCompilerErrorReporter(&TintICEReporter);
    return true;
}

tint::ast::transform::VertexFormat ToTintVertexFormat(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8x2:
            return tint::ast::transform::VertexFormat::kUint8x2;
        case wgpu::VertexFormat::Uint8x4:
            return tint::ast::transform::VertexFormat::kUint8x4;
        case wgpu::VertexFormat::Sint8x2:
            return tint::ast::transform::VertexFormat::kSint8x2;
        case wgpu::VertexFormat::Sint8x4:
            return tint::ast::transform::VertexFormat::kSint8x4;
        case wgpu::VertexFormat::Unorm8x2:
            return tint::ast::transform::VertexFormat::kUnorm8x2;
        case wgpu::VertexFormat::Unorm8x4:
            return tint::ast::transform::VertexFormat::kUnorm8x4;
        case wgpu::VertexFormat::Snorm8x2:
            return tint::ast::transform::VertexFormat::kSnorm8x2;
        case wgpu::VertexFormat::Snorm8x4:
            return tint::ast::transform::VertexFormat::kSnorm8x4;
        case wgpu::VertexFormat::Uint16x2:
            return tint::ast::transform::VertexFormat::kUint16x2;
        case wgpu::VertexFormat::Uint16x4:
            return tint::ast::transform::VertexFormat::kUint16x4;
        case wgpu::VertexFormat::Sint16x2:
            return tint::ast::transform::VertexFormat::kSint16x2;
        case wgpu::VertexFormat::Sint16x4:
            return tint::ast::transform::VertexFormat::kSint16x4;
        case wgpu::VertexFormat::Unorm16x2:
            return tint::ast::transform::VertexFormat::kUnorm16x2;
        case wgpu::VertexFormat::Unorm16x4:
            return tint::ast::transform::VertexFormat::kUnorm16x4;
        case wgpu::VertexFormat::Snorm16x2:
            return tint::ast::transform::VertexFormat::kSnorm16x2;
        case wgpu::VertexFormat::Snorm16x4:
            return tint::ast::transform::VertexFormat::kSnorm16x4;
        case wgpu::VertexFormat::Float16x2:
            return tint::ast::transform::VertexFormat::kFloat16x2;
        case wgpu::VertexFormat::Float16x4:
            return tint::ast::transform::VertexFormat::kFloat16x4;
        case wgpu::VertexFormat::Float32:
            return tint::ast::transform::VertexFormat::kFloat32;
        case wgpu::VertexFormat::Float32x2:
            return tint::ast::transform::VertexFormat::kFloat32x2;
        case wgpu::VertexFormat::Float32x3:
            return tint::ast::transform::VertexFormat::kFloat32x3;
        case wgpu::VertexFormat::Float32x4:
            return tint::ast::transform::VertexFormat::kFloat32x4;
        case wgpu::VertexFormat::Uint32:
            return tint::ast::transform::VertexFormat::kUint32;
        case wgpu::VertexFormat::Uint32x2:
            return tint::ast::transform::VertexFormat::kUint32x2;
        case wgpu::VertexFormat::Uint32x3:
            return tint::ast::transform::VertexFormat::kUint32x3;
        case wgpu::VertexFormat::Uint32x4:
            return tint::ast::transform::VertexFormat::kUint32x4;
        case wgpu::VertexFormat::Sint32:
            return tint::ast::transform::VertexFormat::kSint32;
        case wgpu::VertexFormat::Sint32x2:
            return tint::ast::transform::VertexFormat::kSint32x2;
        case wgpu::VertexFormat::Sint32x3:
            return tint::ast::transform::VertexFormat::kSint32x3;
        case wgpu::VertexFormat::Sint32x4:
            return tint::ast::transform::VertexFormat::kSint32x4;

        case wgpu::VertexFormat::Undefined:
            break;
    }
    UNREACHABLE();
}

tint::ast::transform::VertexStepMode ToTintVertexStepMode(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return tint::ast::transform::VertexStepMode::kVertex;
        case wgpu::VertexStepMode::Instance:
            return tint::ast::transform::VertexStepMode::kInstance;
        case wgpu::VertexStepMode::VertexBufferNotUsed:
            break;
    }
    UNREACHABLE();
}

}  // namespace

ScopedTintICEHandler::ScopedTintICEHandler(DeviceBase* device) {
    // Call tint::SetInternalCompilerErrorReporter() the first time
    // this constructor is called. Static initialization is
    // guaranteed to be thread-safe, and only occur once.
    static bool init_once_tint_error_reporter = InitializeTintErrorReporter();
    (void)init_once_tint_error_reporter;

    // Shouldn't have overlapping instances of this handler.
    ASSERT(tlDevice == nullptr);
    tlDevice = device;
}

ScopedTintICEHandler::~ScopedTintICEHandler() {
    tlDevice = nullptr;
}

tint::ExternalTextureOptions BuildExternalTextureTransformBindings(
    const PipelineLayoutBase* layout) {
    tint::ExternalTextureOptions options;
    for (BindGroupIndex i : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(i);
        for (const auto& [_, expansion] : bgl->GetExternalTextureBindingExpansionMap()) {
            options.bindings_map[{static_cast<uint32_t>(i),
                                  static_cast<uint32_t>(expansion.plane0)}] = {
                {static_cast<uint32_t>(i), static_cast<uint32_t>(expansion.plane1)},
                {static_cast<uint32_t>(i), static_cast<uint32_t>(expansion.params)}};
        }
    }
    return options;
}

tint::ast::transform::VertexPulling::Config BuildVertexPullingTransformConfig(
    const RenderPipelineBase& renderPipeline,
    BindGroupIndex pullingBufferBindingSet) {
    tint::ast::transform::VertexPulling::Config cfg;
    cfg.pulling_group = static_cast<uint32_t>(pullingBufferBindingSet);

    cfg.vertex_state.resize(renderPipeline.GetVertexBufferCount());
    for (VertexBufferSlot slot : IterateBitSet(renderPipeline.GetVertexBufferSlotsUsed())) {
        const VertexBufferInfo& dawnInfo = renderPipeline.GetVertexBuffer(slot);
        tint::ast::transform::VertexBufferLayoutDescriptor* tintInfo =
            &cfg.vertex_state[static_cast<uint8_t>(slot)];

        tintInfo->array_stride = dawnInfo.arrayStride;
        tintInfo->step_mode = ToTintVertexStepMode(dawnInfo.stepMode);
    }

    for (VertexAttributeLocation location :
         IterateBitSet(renderPipeline.GetAttributeLocationsUsed())) {
        const VertexAttributeInfo& dawnInfo = renderPipeline.GetAttribute(location);
        tint::ast::transform::VertexAttributeDescriptor tintInfo;
        tintInfo.format = ToTintVertexFormat(dawnInfo.format);
        tintInfo.offset = dawnInfo.offset;
        tintInfo.shader_location = static_cast<uint32_t>(static_cast<uint8_t>(location));

        uint8_t vertexBufferSlot = static_cast<uint8_t>(dawnInfo.vertexBufferSlot);
        cfg.vertex_state[vertexBufferSlot].attributes.push_back(tintInfo);
    }
    return cfg;
}

tint::ast::transform::SubstituteOverride::Config BuildSubstituteOverridesTransformConfig(
    const ProgrammableStage& stage) {
    const EntryPointMetadata& metadata = *stage.metadata;
    const auto& constants = stage.constants;

    tint::ast::transform::SubstituteOverride::Config cfg;

    for (const auto& [key, value] : constants) {
        const auto& o = metadata.overrides.at(key);
        cfg.map.insert({o.id, value});
    }

    return cfg;
}

}  // namespace dawn::native

namespace tint::sem {

bool operator<(const BindingPoint& a, const BindingPoint& b) {
    return std::tie(a.group, a.binding) < std::tie(b.group, b.binding);
}

}  // namespace tint::sem
