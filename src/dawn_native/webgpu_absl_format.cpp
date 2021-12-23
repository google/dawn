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

#include "dawn_native/webgpu_absl_format.h"

#include "dawn_native/Device.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    //
    // Structs
    //

    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const Color* value,
                      const absl::FormatConversionSpec& spec,
                      absl::FormatSink* s) {
        if (value == nullptr) {
            s->Append("[null]");
            return {true};
        }
        s->Append(absl::StrFormat("[Color r:%f, g:%f, b:%f, a:%f]",
            value->r, value->g, value->b, value->a));
        return {true};
    }

    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const Extent3D* value,
                      const absl::FormatConversionSpec& spec,
                      absl::FormatSink* s) {
        if (value == nullptr) {
            s->Append("[null]");
            return {true};
        }
        s->Append(absl::StrFormat("[Extent3D width:%u, height:%u, depthOrArrayLayers:%u]",
            value->width, value->height, value->depthOrArrayLayers));
        return {true};
    }

    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const Origin3D* value,
                      const absl::FormatConversionSpec& spec,
                      absl::FormatSink* s) {
        if (value == nullptr) {
            s->Append("[null]");
            return {true};
        }
        s->Append(absl::StrFormat("[Origin3D x:%u, y:%u, z:%u]",
            value->x, value->y, value->z));
        return {true};
    }

    //
    // Objects
    //

    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const DeviceBase* value,
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

    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const ApiObjectBase* value,
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

    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const TextureViewBase* value,
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

}  // namespace dawn_native
