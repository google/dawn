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

#include "dawn_native/metal/QuerySetMTL.h"

#include "common/Math.h"
#include "common/Platform.h"
#include "dawn_native/metal/DeviceMTL.h"

namespace dawn_native { namespace metal {

    namespace {

        ResultOrError<id<MTLCounterSampleBuffer>> CreateCounterSampleBuffer(
            Device* device,
            MTLCommonCounterSet counterSet,
            uint32_t count) API_AVAILABLE(macos(10.15), ios(14.0)) {
            MTLCounterSampleBufferDescriptor* descriptor = [MTLCounterSampleBufferDescriptor new];

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
            ASSERT(descriptor.counterSet != nil);
            descriptor.sampleCount = count;
            descriptor.storageMode = MTLStorageModePrivate;
            if (device->IsToggleEnabled(Toggle::MetalUseSharedModeForCounterSampleBuffer)) {
                descriptor.storageMode = MTLStorageModeShared;
            }

            NSError* error = nil;
            id<MTLCounterSampleBuffer> counterSampleBuffer =
                [device->GetMTLDevice() newCounterSampleBufferWithDescriptor:descriptor
                                                                       error:&error];
            if (error != nil) {
                const char* errorString = [error.localizedDescription UTF8String];
                return DAWN_INTERNAL_ERROR(std::string("Error creating query set: ") + errorString);
            }

            return counterSampleBuffer;
        }
    }

    // static
    ResultOrError<QuerySet*> QuerySet::Create(Device* device,
                                              const QuerySetDescriptor* descriptor) {
        Ref<QuerySet> queryset = AcquireRef(new QuerySet(device, descriptor));
        DAWN_TRY(queryset->Initialize());
        return queryset.Detach();
    }

    MaybeError QuerySet::Initialize() {
        Device* device = ToBackend(GetDevice());

        switch (GetQueryType()) {
            case wgpu::QueryType::Occlusion: {
                // Create buffer for writing 64-bit results.
                NSUInteger bufferSize = static_cast<NSUInteger>(GetQueryCount() * sizeof(uint64_t));
                mVisibilityBuffer =
                    [device->GetMTLDevice() newBufferWithLength:bufferSize
                                                        options:MTLResourceStorageModePrivate];
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
        return mVisibilityBuffer;
    }

    id<MTLCounterSampleBuffer> QuerySet::GetCounterSampleBuffer() const
        API_AVAILABLE(macos(10.15), ios(14.0)) {
        return mCounterSampleBuffer;
    }

    QuerySet::~QuerySet() {
        DestroyInternal();
    }

    void QuerySet::DestroyImpl() {
        if (mVisibilityBuffer != nil) {
            [mVisibilityBuffer release];
            mVisibilityBuffer = nil;
        }

        if (@available(macOS 10.15, iOS 14.0, *)) {
            if (mCounterSampleBuffer != nil) {
                [mCounterSampleBuffer release];
                mCounterSampleBuffer = nil;
            }
        }
    }

}}  // namespace dawn_native::metal
