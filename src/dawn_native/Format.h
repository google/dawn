// Copyright 2019 The Dawn Authors
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

#ifndef DAWNNATIVE_FORMAT_H_
#define DAWNNATIVE_FORMAT_H_

#include "dawn_native/dawn_platform.h"

#include "common/ityp_bitset.h"
#include "dawn_native/Error.h"

#include "dawn_native/EnumClassBitmasks.h"

#include <array>

namespace dawn_native {

    enum class Aspect : uint8_t;
    class DeviceBase;

    struct TexelBlockInfo {
        uint32_t blockByteSize;
        uint32_t blockWidth;
        uint32_t blockHeight;
    };

    // The number of formats Dawn knows about. Asserts in BuildFormatTable ensure that this is the
    // exact number of known format.
    static constexpr size_t kKnownFormatCount = 53;

    struct Format;
    using FormatTable = std::array<Format, kKnownFormatCount>;

    // A wgpu::TextureFormat along with all the information about it necessary for validation.
    struct Format {
        enum class Type {
            Float,
            Sint,
            Uint,
            Other,
        };

        wgpu::TextureFormat format;
        bool isRenderable;
        bool isCompressed;
        // A format can be known but not supported because it is part of a disabled extension.
        bool isSupported;
        bool supportsStorageUsage;
        Type type;
        Aspect aspects;

        static Type TextureComponentTypeToFormatType(wgpu::TextureComponentType componentType);
        static wgpu::TextureComponentType FormatTypeToTextureComponentType(Type type);

        bool IsColor() const;
        bool HasDepth() const;
        bool HasStencil() const;
        bool HasDepthOrStencil() const;
        bool HasComponentType(Type componentType) const;

        TexelBlockInfo GetTexelBlockInfo(wgpu::TextureAspect aspect) const;
        TexelBlockInfo GetTexelBlockInfo(Aspect aspect) const;

        // The index of the format in the list of all known formats: a unique number for each format
        // in [0, kKnownFormatCount)
        size_t GetIndex() const;

      private:
        TexelBlockInfo blockInfo;

        friend FormatTable BuildFormatTable(const DeviceBase* device);
    };

    // Implementation details of the format table in the device.

    // Returns the index of a format in the FormatTable.
    size_t ComputeFormatIndex(wgpu::TextureFormat format);
    // Builds the format table with the extensions enabled on the device.
    FormatTable BuildFormatTable(const DeviceBase* device);

}  // namespace dawn_native

#endif  // DAWNNATIVE_FORMAT_H_
