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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/WGPUHelpers.h"

class QuerySetValidationTest : public ValidationTest {
  protected:
    wgpu::QuerySet CreateQuerySet(
        wgpu::Device cDevice,
        wgpu::QueryType queryType,
        uint32_t queryCount,
        std::vector<wgpu::PipelineStatisticName> pipelineStatistics = {}) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.type = queryType;
        descriptor.count = queryCount;

        if (pipelineStatistics.size() > 0) {
            descriptor.pipelineStatistics = pipelineStatistics.data();
            descriptor.pipelineStatisticsCount = pipelineStatistics.size();
        }

        return cDevice.CreateQuerySet(&descriptor);
    }
};

// Test creating query set without extensions
TEST_F(QuerySetValidationTest, CreationWithoutExtensions) {
    // Creating a query set for occlusion queries succeeds without any extensions enabled.
    CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

    // Creating a query set for other types of queries fails without extensions enabled.
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                                       {wgpu::PipelineStatisticName::VertexShaderInvocations}));
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Timestamp, 1));
}

// Test creating query set with invalid count
TEST_F(QuerySetValidationTest, InvalidQueryCount) {
    // Success create a query set with the maximum count
    CreateQuerySet(device, wgpu::QueryType::Occlusion, kMaxQueryCount);

    // Fail to create a query set with the count which exceeds the maximum
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Occlusion, kMaxQueryCount + 1));
}

// Test creating query set with invalid type
TEST_F(QuerySetValidationTest, InvalidQueryType) {
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, static_cast<wgpu::QueryType>(0xFFFFFFFF), 1));
}

// Test creating query set with unnecessary pipeline statistics for occlusion queries
TEST_F(QuerySetValidationTest, UnnecessaryPipelineStatistics) {
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Occlusion, 1,
                                       {wgpu::PipelineStatisticName::VertexShaderInvocations}));
}

// Test destroying a destroyed query set
TEST_F(QuerySetValidationTest, DestroyDestroyedQuerySet) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.type = wgpu::QueryType::Occlusion;
    descriptor.count = 1;
    wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
    querySet.Destroy();
    querySet.Destroy();
}

class OcclusionQueryValidationTest : public QuerySetValidationTest {};

// Test the occlusionQuerySet in RenderPassDescriptor
TEST_F(OcclusionQueryValidationTest, InvalidOcclusionQuerySet) {
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);
    DummyRenderPass renderPass(device);

    // Success
    {
        renderPass.occlusionQuerySet = occlusionQuerySet;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.BeginOcclusionQuery(1);
        pass.EndOcclusionQuery();
        pass.EndPass();
        encoder.Finish();
    }

    // Fail to begin occlusion query if the occlusionQuerySet is not set in RenderPassDescriptor
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        DummyRenderPass renderPassWithoutOcclusion(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassWithoutOcclusion);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to begin render pass if the occlusionQuerySet is created from other device
    {
        wgpu::Device otherDevice = RegisterDevice(adapter.CreateDevice());
        wgpu::QuerySet occlusionQuerySetOnOther =
            CreateQuerySet(otherDevice, wgpu::QueryType::Occlusion, 2);
        renderPass.occlusionQuerySet = occlusionQuerySetOnOther;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());

        // Clear this out so we don't hold a reference. The query set
        // must be destroyed before the device local to this test case.
        renderPass.occlusionQuerySet = wgpu::QuerySet();
    }

    // Fail to submit occlusion query with a destroyed query set
    {
        renderPass.occlusionQuerySet = occlusionQuerySet;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.EndPass();
        wgpu::CommandBuffer commands = encoder.Finish();
        wgpu::Queue queue = device.GetDefaultQueue();
        occlusionQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test query index of occlusion query
TEST_F(OcclusionQueryValidationTest, InvalidQueryIndex) {
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);
    DummyRenderPass renderPass(device);
    renderPass.occlusionQuerySet = occlusionQuerySet;

    // Fail to begin occlusion query if the query index exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(2);
        pass.EndOcclusionQuery();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success to begin occlusion query with same query index twice on different render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass);
        pass0.BeginOcclusionQuery(0);
        pass0.EndOcclusionQuery();
        pass0.EndPass();

        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass);
        pass1.BeginOcclusionQuery(0);
        pass1.EndOcclusionQuery();
        pass1.EndPass();
        encoder.Finish();
    }

    // Fail to begin occlusion query with same query index twice on a same render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test the correspondence between BeginOcclusionQuery and EndOcclusionQuery
TEST_F(OcclusionQueryValidationTest, InvalidBeginAndEnd) {
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);
    DummyRenderPass renderPass(device);
    renderPass.occlusionQuerySet = occlusionQuerySet;

    // Fail to begin an occlusion query without corresponding end operation
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.BeginOcclusionQuery(1);
        pass.EndOcclusionQuery();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to end occlusion query twice in a row even the begin occlusion query twice
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.BeginOcclusionQuery(1);
        pass.EndOcclusionQuery();
        pass.EndOcclusionQuery();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to end occlusion query without begin operation
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.EndOcclusionQuery();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

class TimestampQueryValidationTest : public QuerySetValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        dawn_native::DeviceDescriptor descriptor;
        descriptor.requiredExtensions = {"timestamp_query"};
        return adapter.CreateDevice(&descriptor);
    }
};

// Test creating query set with only the timestamp extension enabled.
TEST_F(TimestampQueryValidationTest, Creation) {
    // Creating a query set for occlusion queries succeeds.
    CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

    // Creating a query set for pipeline statistics queries fails.
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                                       {wgpu::PipelineStatisticName::VertexShaderInvocations}));

    // Creating a query set for timestamp queries succeeds.
    CreateQuerySet(device, wgpu::QueryType::Timestamp, 1);

    // Fail to create with pipeline statistics for Timestamp query
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Timestamp, 1,
                                       {wgpu::PipelineStatisticName::VertexShaderInvocations}));
}

// Test creating query set with unnecessary pipeline statistics for timestamp queries
TEST_F(TimestampQueryValidationTest, UnnecessaryPipelineStatistics) {
    // Fail to create with pipeline statistics for Occlusion query
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Timestamp, 1,
                                       {wgpu::PipelineStatisticName::VertexShaderInvocations}));
}

// Test query set with type of timestamp is set to the occlusionQuerySet of RenderPassDescriptor.
TEST_F(TimestampQueryValidationTest, BeginRenderPassWithTimestampQuerySet) {
    // Fail to begin render pass if the type of occlusionQuerySet is not Occlusion
    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 1);
    DummyRenderPass renderPass(device);
    renderPass.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPass);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test write timestamp on command encoder
TEST_F(TimestampQueryValidationTest, WriteTimestampOnCommandEncoder) {
    wgpu::QuerySet timestampQuerySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);

    // Success on command encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 0);
        encoder.Finish();
    }

    // Fail to write timestamp to the index which exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 2);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to submit timestamp query with a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        timestampQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test write timestamp on compute pass encoder
TEST_F(TimestampQueryValidationTest, WriteTimestampOnComputePassEncoder) {
    wgpu::QuerySet timestampQuerySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);

    // Success on compute pass encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.EndPass();
        encoder.Finish();
    }

    // Not allow to write timestamp to the query set with other query type
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(occlusionQuerySet, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamp to the index which exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(timestampQuerySet, 2);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to submit timestamp query with a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.EndPass();
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        timestampQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test write timestamp on render pass encoder
TEST_F(TimestampQueryValidationTest, WriteTimestampOnRenderPassEncoder) {
    DummyRenderPass renderPass(device);

    wgpu::QuerySet timestampQuerySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);

    // Success on render pass encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.EndPass();
        encoder.Finish();
    }

    // Not allow to write timestamp to the query set with other query type
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(occlusionQuerySet, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamp to the index which exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 2);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success to write timestamp to the same query index twice on command encoder and render
    // encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 0);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.EndPass();
        encoder.Finish();
    }

    // Success to write timestamp to the same query index twice on different render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass);
        pass0.WriteTimestamp(timestampQuerySet, 0);
        pass0.EndPass();
        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass);
        pass1.WriteTimestamp(timestampQuerySet, 0);
        pass1.EndPass();
        encoder.Finish();
    }

    // Fail to write timestamp to the same query index twice on same render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to submit timestamp query with a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.EndPass();
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        timestampQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

class PipelineStatisticsQueryValidationTest : public QuerySetValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        dawn_native::DeviceDescriptor descriptor;
        descriptor.requiredExtensions = {"pipeline_statistics_query"};
        return adapter.CreateDevice(&descriptor);
    }
};

// Test creating query set with only the pipeline statistics extension enabled.
TEST_F(PipelineStatisticsQueryValidationTest, Creation) {
    // Creating a query set for occlusion queries succeeds.
    CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

    // Creating a query set for timestamp queries fails.
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Timestamp, 1));

    // Creating a query set for pipeline statistics queries succeeds.
    CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                   {wgpu::PipelineStatisticName::VertexShaderInvocations});
}

// Test creating query set with invalid pipeline statistics
TEST_F(PipelineStatisticsQueryValidationTest, InvalidPipelineStatistics) {
    // Success to create with all pipeline statistics names which are not in the same order as
    // defined in webgpu header file
    {
        CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                       {wgpu::PipelineStatisticName::ClipperInvocations,
                        wgpu::PipelineStatisticName::ClipperPrimitivesOut,
                        wgpu::PipelineStatisticName::ComputeShaderInvocations,
                        wgpu::PipelineStatisticName::FragmentShaderInvocations,
                        wgpu::PipelineStatisticName::VertexShaderInvocations});
    }

    // Fail to create with empty pipeline statistics
    { ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1, {})); }

    // Fail to create with invalid pipeline statistics
    {
        ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                                           {static_cast<wgpu::PipelineStatisticName>(0xFFFFFFFF)}));
    }

    // Fail to create with duplicate pipeline statistics
    {
        ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                                           {wgpu::PipelineStatisticName::VertexShaderInvocations,
                                            wgpu::PipelineStatisticName::VertexShaderInvocations}));
    }
}

// Test query set with type of pipeline statistics is set to the occlusionQuerySet of
// RenderPassDescriptor.
TEST_F(PipelineStatisticsQueryValidationTest, BeginRenderPassWithPipelineStatisticsQuerySet) {
    // Fail to begin render pass if the type of occlusionQuerySet is not Occlusion
    wgpu::QuerySet querySet =
        CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                       {wgpu::PipelineStatisticName::VertexShaderInvocations});
    DummyRenderPass renderPass(device);
    renderPass.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPass);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

class ResolveQuerySetValidationTest : public QuerySetValidationTest {
  protected:
    wgpu::Buffer CreateBuffer(wgpu::Device cDevice, uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;

        return cDevice.CreateBuffer(&descriptor);
    }
};

// Test resolve query set with invalid query set, first query and query count
TEST_F(ResolveQuerySetValidationTest, ResolveInvalidQuerySetAndIndexCount) {
    constexpr uint32_t kQueryCount = 4;

    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, kQueryCount);
    wgpu::Buffer destination =
        CreateBuffer(device, kQueryCount * sizeof(uint64_t), wgpu::BufferUsage::QueryResolve);

    // Success
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        queue.Submit(1, &commands);
    }

    //  Fail to resolve query set if first query out of range
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, kQueryCount, 0, destination, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set if the sum of first query and query count is larger than queries
    //  number in the query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 1, kQueryCount, destination, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to resolve a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        querySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test resolve query set with invalid query set, first query and query count
TEST_F(ResolveQuerySetValidationTest, ResolveToInvalidBufferAndOffset) {
    constexpr uint32_t kQueryCount = 4;
    constexpr uint64_t kBufferSize = kQueryCount * sizeof(uint64_t);

    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, kQueryCount);
    wgpu::Buffer destination = CreateBuffer(device, kBufferSize, wgpu::BufferUsage::QueryResolve);

    // Success
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 1, kQueryCount - 1, destination, 8);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        queue.Submit(1, &commands);
    }

    // Fail to resolve query set to a buffer created from another device
    {
        wgpu::Device otherDevice = RegisterDevice(adapter.CreateDevice());
        wgpu::Buffer bufferOnOther =
            CreateBuffer(otherDevice, kBufferSize, wgpu::BufferUsage::QueryResolve);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, bufferOnOther, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer if offset is not a multiple of 8 bytes
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 4);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer if the data size overflow the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 8);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer if the offset is past the end of the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, 1, destination, kBufferSize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer does not have the usage of QueryResolve
    {
        wgpu::Buffer dstBuffer = CreateBuffer(device, kBufferSize, wgpu::BufferUsage::CopyDst);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, dstBuffer, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to resolve query set to a destroyed buffer.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetDefaultQueue();
        destination.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}
