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

#include "dawn/tests/unittests/native/mocks/DeviceMock.h"

#include "dawn/tests/unittests/native/mocks/BindGroupLayoutMock.h"
#include "dawn/tests/unittests/native/mocks/BindGroupMock.h"
#include "dawn/tests/unittests/native/mocks/BufferMock.h"
#include "dawn/tests/unittests/native/mocks/CommandBufferMock.h"
#include "dawn/tests/unittests/native/mocks/ComputePipelineMock.h"
#include "dawn/tests/unittests/native/mocks/ExternalTextureMock.h"
#include "dawn/tests/unittests/native/mocks/PipelineLayoutMock.h"
#include "dawn/tests/unittests/native/mocks/QuerySetMock.h"
#include "dawn/tests/unittests/native/mocks/QueueMock.h"
#include "dawn/tests/unittests/native/mocks/RenderPipelineMock.h"
#include "dawn/tests/unittests/native/mocks/SamplerMock.h"
#include "dawn/tests/unittests/native/mocks/ShaderModuleMock.h"
#include "dawn/tests/unittests/native/mocks/TextureMock.h"

namespace dawn::native {

using ::testing::NiceMock;
using ::testing::WithArgs;

DeviceMock::DeviceMock() {
    mInstance = InstanceBase::Create();

    // Set all default creation functions to return nice mock objects.
    ON_CALL(*this, CreateBindGroupImpl)
        .WillByDefault(WithArgs<0>(
            [this](const BindGroupDescriptor* descriptor) -> ResultOrError<Ref<BindGroupBase>> {
                return AcquireRef(new NiceMock<BindGroupMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateBindGroupLayoutImpl)
        .WillByDefault(WithArgs<0>([this](const BindGroupLayoutDescriptor* descriptor)
                                       -> ResultOrError<Ref<BindGroupLayoutInternalBase>> {
            return AcquireRef(new NiceMock<BindGroupLayoutMock>(this, descriptor));
        }));
    ON_CALL(*this, CreateBufferImpl)
        .WillByDefault(WithArgs<0>(
            [this](const BufferDescriptor* descriptor) -> ResultOrError<Ref<BufferBase>> {
                return AcquireRef(new NiceMock<BufferMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateCommandBuffer)
        .WillByDefault(WithArgs<0, 1>(
            [this](CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
                -> ResultOrError<Ref<CommandBufferBase>> {
                return AcquireRef(new NiceMock<CommandBufferMock>(this, encoder, descriptor));
            }));
    ON_CALL(*this, CreateExternalTextureImpl)
        .WillByDefault(WithArgs<0>([this](const ExternalTextureDescriptor* descriptor)
                                       -> ResultOrError<Ref<ExternalTextureBase>> {
            return ExternalTextureMock::Create(this, descriptor);
        }));
    ON_CALL(*this, CreatePipelineLayoutImpl)
        .WillByDefault(WithArgs<0>([this](const PipelineLayoutDescriptor* descriptor)
                                       -> ResultOrError<Ref<PipelineLayoutBase>> {
            return AcquireRef(new NiceMock<PipelineLayoutMock>(this, descriptor));
        }));
    ON_CALL(*this, CreateQuerySetImpl)
        .WillByDefault(WithArgs<0>(
            [this](const QuerySetDescriptor* descriptor) -> ResultOrError<Ref<QuerySetBase>> {
                return AcquireRef(new NiceMock<QuerySetMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateSamplerImpl)
        .WillByDefault(WithArgs<0>(
            [this](const SamplerDescriptor* descriptor) -> ResultOrError<Ref<SamplerBase>> {
                return AcquireRef(new NiceMock<SamplerMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateShaderModuleImpl)
        .WillByDefault(WithArgs<0>([this](const ShaderModuleDescriptor* descriptor)
                                       -> ResultOrError<Ref<ShaderModuleBase>> {
            return ShaderModuleMock::Create(this, descriptor);
        }));
    ON_CALL(*this, CreateTextureImpl)
        .WillByDefault(WithArgs<0>(
            [this](const TextureDescriptor* descriptor) -> ResultOrError<Ref<TextureBase>> {
                return AcquireRef(new NiceMock<TextureMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateTextureViewImpl)
        .WillByDefault(WithArgs<0, 1>(
            [](TextureBase* texture,
               const TextureViewDescriptor* descriptor) -> ResultOrError<Ref<TextureViewBase>> {
                return AcquireRef(new NiceMock<TextureViewMock>(texture, descriptor));
            }));
    ON_CALL(*this, CreateUninitializedComputePipelineImpl)
        .WillByDefault(WithArgs<0>(
            [this](const ComputePipelineDescriptor* descriptor) -> Ref<ComputePipelineBase> {
                return ComputePipelineMock::Create(this, descriptor);
            }));
    ON_CALL(*this, CreateUninitializedRenderPipelineImpl)
        .WillByDefault(WithArgs<0>(
            [this](const RenderPipelineDescriptor* descriptor) -> Ref<RenderPipelineBase> {
                return RenderPipelineMock::Create(this, descriptor);
            }));

    // By default, the mock's TickImpl will succeed.
    ON_CALL(*this, TickImpl).WillByDefault([]() -> MaybeError { return {}; });

    // Initialize the device.
    QueueDescriptor desc = {};
    EXPECT_FALSE(Initialize(AcquireRef(new NiceMock<QueueMock>(this, &desc))).IsError());
}

DeviceMock::~DeviceMock() = default;

dawn::platform::Platform* DeviceMock::GetPlatform() const {
    return mInstance->GetPlatform();
}

QueueMock* DeviceMock::GetQueueMock() {
    return reinterpret_cast<QueueMock*>(GetQueue());
}

}  // namespace dawn::native
