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

#include "dawn/native/webgpu_absl_format.h"

#include <string>

#include "dawn/native/AttachmentState.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Device.h"
#include "dawn/native/Format.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/Subresource.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Texture.h"
#include "dawn/native/VertexFormat.h"

namespace dawn::native {

//
// Structs
//

absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(const Color* value, const absl::FormatConversionSpec& spec, absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(
        absl::StrFormat("[Color r:%f, g:%f, b:%f, a:%f]", value->r, value->g, value->b, value->a));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Extent2D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Extent2D width:%u, height:%u]", value->width, value->height));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Extent3D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Extent3D width:%u, height:%u, depthOrArrayLayers:%u]", value->width,
                              value->height, value->depthOrArrayLayers));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Origin2D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Origin2D x:%u, y:%u]", value->x, value->y));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Origin3D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Origin3D x:%u, y:%u, z:%u]", value->x, value->y, value->z));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    static const auto* const fmt =
        new absl::ParsedFormat<'u', 's', 's', 's'>("{ binding: %u, visibility: %s, %s: %s }");
    switch (value.bindingType) {
        case BindingInfoType::Buffer:
            s->Append(absl::StrFormat(*fmt, static_cast<uint32_t>(value.binding), value.visibility,
                                      value.bindingType, value.buffer));
            break;
        case BindingInfoType::Sampler:
            s->Append(absl::StrFormat(*fmt, static_cast<uint32_t>(value.binding), value.visibility,
                                      value.bindingType, value.sampler));
            break;
        case BindingInfoType::Texture:
            s->Append(absl::StrFormat(*fmt, static_cast<uint32_t>(value.binding), value.visibility,
                                      value.bindingType, value.texture));
            break;
        case BindingInfoType::StorageTexture:
            s->Append(absl::StrFormat(*fmt, static_cast<uint32_t>(value.binding), value.visibility,
                                      value.bindingType, value.storageTexture));
            break;
        case BindingInfoType::ExternalTexture:
            break;
    }
    return {true};
}

//
// Objects
//

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const DeviceBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[Device");
    const std::string& label = value->GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
    s->Append("]");
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const ApiObjectBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[");
    if (value->IsError()) {
        s->Append("Invalid ");
    }
    s->Append(ObjectTypeAsString(value->GetType()));
    const std::string& label = value->GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
    s->Append("]");
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TextureViewBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[");
    if (value->IsError()) {
        s->Append("Invalid ");
    }
    s->Append(ObjectTypeAsString(value->GetType()));
    const std::string& label = value->GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
    const std::string& textureLabel = value->GetTexture()->GetLabel();
    if (!textureLabel.empty()) {
        s->Append(absl::StrFormat(" of Texture \"%s\"", textureLabel));
    }
    s->Append("]");
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const AttachmentState* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }

    s->Append("{ colorFormats: [");

    ColorAttachmentIndex nextColorIndex(uint8_t(0));

    bool needsComma = false;
    for (ColorAttachmentIndex i : IterateBitSet(value->GetColorAttachmentsMask())) {
        if (needsComma) {
            s->Append(", ");
        }

        while (nextColorIndex < i) {
            s->Append(absl::StrFormat("%s, ", wgpu::TextureFormat::Undefined));
            nextColorIndex++;
            needsComma = false;
        }

        s->Append(absl::StrFormat("%s", value->GetColorAttachmentFormat(i)));

        nextColorIndex++;
        needsComma = true;
    }

    s->Append("], ");

    if (value->HasDepthStencilAttachment()) {
        s->Append(absl::StrFormat("depthStencilFormat: %s, ", value->GetDepthStencilFormat()));
    }

    s->Append(absl::StrFormat("sampleCount: %u }", value->GetSampleCount()));

    return {true};
}

//
// Enums
//

absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(Aspect value, const absl::FormatConversionSpec& spec, absl::FormatSink* s) {
    if (value == Aspect::None) {
        s->Append("None");
        return {true};
    }

    bool first = true;

    if (value & Aspect::Color) {
        first = false;
        s->Append("Color");
        value &= ~Aspect::Color;
    }

    if (value & Aspect::Depth) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Depth");
        value &= ~Aspect::Depth;
    }

    if (value & Aspect::Stencil) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Stencil");
        value &= ~Aspect::Stencil;
    }

    // Output any remaining flags as a hex value
    if (static_cast<bool>(value)) {
        if (!first) {
            s->Append("|");
        }
        s->Append(absl::StrFormat("%x", static_cast<uint8_t>(value)));
    }

    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    SampleTypeBit value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == SampleTypeBit::None) {
        s->Append("None");
        return {true};
    }

    bool first = true;

    if (value & SampleTypeBit::Float) {
        first = false;
        s->Append("Float");
        value &= ~SampleTypeBit::Float;
    }

    if (value & SampleTypeBit::UnfilterableFloat) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("UnfilterableFloat");
        value &= ~SampleTypeBit::UnfilterableFloat;
    }

    if (value & SampleTypeBit::Depth) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Depth");
        value &= ~SampleTypeBit::Depth;
    }

    if (value & SampleTypeBit::Sint) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Sint");
        value &= ~SampleTypeBit::Sint;
    }

    if (value & SampleTypeBit::Uint) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Uint");
        value &= ~SampleTypeBit::Uint;
    }

    // Output any remaining flags as a hex value
    if (static_cast<bool>(value)) {
        if (!first) {
            s->Append("|");
        }
        s->Append(absl::StrFormat("%x", static_cast<uint8_t>(value)));
    }

    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    BindingInfoType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case BindingInfoType::Buffer:
            s->Append("buffer");
            break;
        case BindingInfoType::Sampler:
            s->Append("sampler");
            break;
        case BindingInfoType::Texture:
            s->Append("texture");
            break;
        case BindingInfoType::StorageTexture:
            s->Append("storageTexture");
            break;
        case BindingInfoType::ExternalTexture:
            s->Append("externalTexture");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    SingleShaderStage value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case SingleShaderStage::Compute:
            s->Append("Compute");
            break;
        case SingleShaderStage::Vertex:
            s->Append("Vertex");
            break;
        case SingleShaderStage::Fragment:
            s->Append("Fragment");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    VertexFormatBaseType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case VertexFormatBaseType::Float:
            s->Append("Float");
            break;
        case VertexFormatBaseType::Uint:
            s->Append("Uint");
            break;
        case VertexFormatBaseType::Sint:
            s->Append("Sint");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterStageComponentType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case InterStageComponentType::F32:
            s->Append("f32");
            break;
        case InterStageComponentType::F16:
            s->Append("f16");
            break;
        case InterStageComponentType::U32:
            s->Append("u32");
            break;
        case InterStageComponentType::I32:
            s->Append("i32");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterpolationType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case InterpolationType::Perspective:
            s->Append("Perspective");
            break;
        case InterpolationType::Linear:
            s->Append("Linear");
            break;
        case InterpolationType::Flat:
            s->Append("Flat");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterpolationSampling value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case InterpolationSampling::None:
            s->Append("None");
            break;
        case InterpolationSampling::Center:
            s->Append("Center");
            break;
        case InterpolationSampling::Centroid:
            s->Append("Centroid");
            break;
        case InterpolationSampling::Sample:
            s->Append("Sample");
            break;
    }
    return {true};
}

}  // namespace dawn::native
