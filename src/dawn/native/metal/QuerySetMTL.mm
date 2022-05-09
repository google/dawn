// Copyright 2020 The Dawn Authors
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

#include "dawn/native/metal/QuerySetMTL.h"

#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/native/metal/DeviceMTL.h"

namespace dawn::native::metal {

namespace {

ResultOrError<id<MTLCounterSampleBuffer>> CreateCounterSampleBuffer(Device* device,
                                                                    MTLCommonCounterSet counterSet,
                                                                    uint32_t count)
    API_AVAILABLE(macos(10.15), ios(14.0)) {
    NSRef<MTLCounterSampleBufferDescriptor> descriptorRef =
        AcquireNSRef([MTLCounterSampleBufferDescriptor new]);
    MTLCounterSampleBufferDescriptor* descriptor = descriptorRef.Get();

    // To determine which counters are available from a device, we need to iterate through
    // the counterSets property of a MTLDevice. Then configure which counters will be
    // sampled by creating a MTLCounterSampleBufferDescriptor and setting its counterSet
    // property to the matched one of the available set.
    for (id<MTLCounterSet> set in device->GetMTLDevice().counterSets) {
        if ([set.name isEqualToString:counterSet]) {
            descriptor.counterSet = set;
            break;
        }
    }
    ASSERT(descriptor.counterSet != nullptr);

    descriptor.sampleCount = static_cast<NSUInteger>(std::max(count, uint32_t(1u)));
    descriptor.storageMode = MTLStorageModePrivate;
    if (device->IsToggleEnabled(Toggle::MetalUseSharedModeForCounterSampleBuffer)) {
        descriptor.storageMode = MTLStorageModeShared;
    }

    NSError* error = nullptr;
    id<MTLCounterSampleBuffer> counterSampleBuffer =
        [device->GetMTLDevice() newCounterSampleBufferWithDescriptor:descriptor error:&error];
    if (error != nullptr) {
        return DAWN_OUT_OF_MEMORY_ERROR(std::string("Error creating query set: ") +
                                        [error.localizedDescription UTF8String]);
    }

    return counterSampleBuffer;
}
}  // namespace

// static
ResultOrError<Ref<QuerySet>> QuerySet::Create(Device* device,
                                              const QuerySetDescriptor* descriptor) {
    Ref<QuerySet> queryset = AcquireRef(new QuerySet(device, descriptor));
    DAWN_TRY(queryset->Initialize());
    return queryset;
}

QuerySet::QuerySet(DeviceBase* dev, const QuerySetDescriptor* desc) : QuerySetBase(dev, desc) {}

MaybeError QuerySet::Initialize() {
    Device* device = ToBackend(GetDevice());

    switch (GetQueryType()) {
        case wgpu::QueryType::Occlusion: {
            // Create buffer for writing 64-bit results.
            NSUInteger bufferSize =
                static_cast<NSUInteger>(std::max(GetQueryCount() * sizeof(uint64_t), size_t(4u)));
            mVisibilityBuffer = AcquireNSPRef([device->GetMTLDevice()
                newBufferWithLength:bufferSize
                            options:MTLResourceStorageModePrivate]);

            if (mVisibilityBuffer == nil) {
                return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate query set.");
            }
            break;
        }
        case wgpu::QueryType::PipelineStatistics:
            if (@available(macOS 10.15, iOS 14.0, *)) {
                DAWN_TRY_ASSIGN(mCounterSampleBuffer,
                                CreateCounterSampleBuffer(device, MTLCommonCounterSetStatistic,
                                                          GetQueryCount()));
            } else {
                UNREACHABLE();
            }
            break;
        case wgpu::QueryType::Timestamp:
            if (@available(macOS 10.15, iOS 14.0, *)) {
                DAWN_TRY_ASSIGN(mCounterSampleBuffer,
                                CreateCounterSampleBuffer(device, MTLCommonCounterSetTimestamp,
                                                          GetQueryCount()));
            } else {
                UNREACHABLE();
            }
            break;
        default:
            UNREACHABLE();
            break;
    }

    return {};
}

id<MTLBuffer> QuerySet::GetVisibilityBuffer() const {
    return mVisibilityBuffer.Get();
}

id<MTLCounterSampleBuffer> QuerySet::GetCounterSampleBuffer() const
    API_AVAILABLE(macos(10.15), ios(14.0)) {
    return mCounterSampleBuffer;
}

QuerySet::~QuerySet() = default;

void QuerySet::DestroyImpl() {
    QuerySetBase::DestroyImpl();

    mVisibilityBuffer = nullptr;

    // mCounterSampleBuffer isn't an NSRef because API_AVAILABLE doesn't work will with
    // templates.
    if (@available(macOS 10.15, iOS 14.0, *)) {
        [mCounterSampleBuffer release];
        mCounterSampleBuffer = nullptr;
    }
}

}  // namespace dawn::native::metal
