// Copyright 2023 The Dawn Authors
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

#include "dawn/common/IOSurfaceUtils.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>

#include "dawn/common/Assert.h"
#include "dawn/common/CoreFoundationRef.h"

namespace dawn {

namespace {

void AddIntegerValue(CFMutableDictionaryRef dictionary, const CFStringRef key, int32_t value) {
    CFRef<CFNumberRef> number = AcquireCFRef(CFNumberCreate(nullptr, kCFNumberSInt32Type, &value));
    CFDictionaryAddValue(dictionary, key, number.Get());
}

OSType ToCVFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

uint32_t NumPlanes(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return 2;
        default:
            DAWN_UNREACHABLE();
            return 1;
    }
}

size_t GetSubSamplingFactorPerPlane(wgpu::TextureFormat format, size_t plane) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return plane == 0 ? 1 : 2;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

size_t BytesPerElement(wgpu::TextureFormat format, size_t plane) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return plane == 0 ? 1 : 2;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return plane == 0 ? 2 : 4;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

}  // namespace

IOSurfaceRef CreateMultiPlanarIOSurface(wgpu::TextureFormat format,
                                        uint32_t width,
                                        uint32_t height) {
    CFRef<CFMutableDictionaryRef> dict = AcquireCFRef(CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    AddIntegerValue(dict.Get(), kIOSurfaceWidth, width);
    AddIntegerValue(dict.Get(), kIOSurfaceHeight, height);
    AddIntegerValue(dict.Get(), kIOSurfacePixelFormat, ToCVFormat(format));

    size_t numPlanes = NumPlanes(format);

    CFRef<CFMutableArrayRef> planes =
        AcquireCFRef(CFArrayCreateMutable(kCFAllocatorDefault, numPlanes, &kCFTypeArrayCallBacks));
    size_t totalBytesAlloc = 0;
    for (size_t plane = 0; plane < numPlanes; ++plane) {
        const size_t factor = GetSubSamplingFactorPerPlane(format, plane);
        const size_t planeWidth = width / factor;
        const size_t planeHeight = height / factor;
        const size_t planeBytesPerElement = BytesPerElement(format, plane);
        const size_t planeBytesPerRow =
            IOSurfaceAlignProperty(kIOSurfacePlaneBytesPerRow, planeWidth * planeBytesPerElement);
        const size_t planeBytesAlloc =
            IOSurfaceAlignProperty(kIOSurfacePlaneSize, planeHeight * planeBytesPerRow);
        const size_t planeOffset = IOSurfaceAlignProperty(kIOSurfacePlaneOffset, totalBytesAlloc);

        CFRef<CFMutableDictionaryRef> planeInfo = AcquireCFRef(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneWidth, planeWidth);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneHeight, planeHeight);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneBytesPerElement, planeBytesPerElement);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneBytesPerRow, planeBytesPerRow);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneSize, planeBytesAlloc);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneOffset, planeOffset);
        CFArrayAppendValue(planes.Get(), planeInfo.Get());
        totalBytesAlloc = planeOffset + planeBytesAlloc;
    }
    CFDictionaryAddValue(dict.Get(), kIOSurfacePlaneInfo, planes.Get());

    totalBytesAlloc = IOSurfaceAlignProperty(kIOSurfaceAllocSize, totalBytesAlloc);
    AddIntegerValue(dict.Get(), kIOSurfaceAllocSize, totalBytesAlloc);

    IOSurfaceRef surface = IOSurfaceCreate(dict.Get());

    return surface;
}
}  // namespace dawn
