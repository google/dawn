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
    void SetUp() override {
        ValidationTest::SetUp();

        // Initialize the device with required extensions
        deviceWithPipelineStatistics =
            CreateDeviceFromAdapter(adapter, {"pipeline_statistics_query"});
        deviceWithTimestamp = CreateDeviceFromAdapter(adapter, {"timestamp_query"});
    }

    void CreateQuerySet(wgpu::Device cDevice,
                        wgpu::QueryType queryType,
                        uint32_t queryCount,
                        std::vector<wgpu::PipelineStatisticsName> pipelineStatistics = {}) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.type = queryType;
        descriptor.count = queryCount;

        if (pipelineStatistics.size() > 0) {
            descriptor.pipelineStatistics = pipelineStatistics.data();
            descriptor.pipelineStatisticsCount = pipelineStatistics.size();
        }

        cDevice.CreateQuerySet(&descriptor);
    }

    wgpu::Device deviceWithPipelineStatistics;
    wgpu::Device deviceWithTimestamp;
};

// Test creating query set with/without extensions
TEST_F(QuerySetValidationTest, Creation) {
    // Create query set for Occlusion query
    {
        // Success on default device without any extension enabled
        // Occlusion query does not require any extension.
        CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

        // Success on the device with extension enabled.
        CreateQuerySet(deviceWithPipelineStatistics, wgpu::QueryType::Occlusion, 1);
        CreateQuerySet(deviceWithTimestamp, wgpu::QueryType::Occlusion, 1);
    }

    // Create query set for PipelineStatistics query
    {
        // Fail on default device without any extension enabled
        ASSERT_DEVICE_ERROR(
            CreateQuerySet(device, wgpu::QueryType::PipelineStatistics, 1,
                           {wgpu::PipelineStatisticsName::VertexShaderInvocations}));

        // Success on the device if the extension is enabled.
        CreateQuerySet(deviceWithPipelineStatistics, wgpu::QueryType::PipelineStatistics, 1,
                       {wgpu::PipelineStatisticsName::VertexShaderInvocations});
    }

    // Create query set for Timestamp query
    {
        // Fail on default device without any extension enabled
        ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Timestamp, 1));

        // Success on the device if the extension is enabled.
        CreateQuerySet(deviceWithTimestamp, wgpu::QueryType::Timestamp, 1);
    }
}

// Test creating query set with invalid type
TEST_F(QuerySetValidationTest, InvalidQueryType) {
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, static_cast<wgpu::QueryType>(0xFFFFFFFF), 1));
}

// Test creating query set with unnecessary pipeline statistics
TEST_F(QuerySetValidationTest, UnnecessaryPipelineStatistics) {
    // Fail to create with pipeline statistics for Occlusion query
    {
        ASSERT_DEVICE_ERROR(
            CreateQuerySet(device, wgpu::QueryType::Occlusion, 1,
                           {wgpu::PipelineStatisticsName::VertexShaderInvocations}));
    }

    // Fail to create with pipeline statistics for Timestamp query
    {
        ASSERT_DEVICE_ERROR(
            CreateQuerySet(deviceWithTimestamp, wgpu::QueryType::Timestamp, 1,
                           {wgpu::PipelineStatisticsName::VertexShaderInvocations}));
    }
}

// Test creating query set with invalid pipeline statistics
TEST_F(QuerySetValidationTest, InvalidPipelineStatistics) {
    // Success to create with all pipeline statistics names which are not in the same order as
    // defined in webgpu header file
    {
        CreateQuerySet(deviceWithPipelineStatistics, wgpu::QueryType::PipelineStatistics, 1,
                       {wgpu::PipelineStatisticsName::ClipperInvocations,
                        wgpu::PipelineStatisticsName::ClipperPrimitivesOut,
                        wgpu::PipelineStatisticsName::ComputeShaderInvocations,
                        wgpu::PipelineStatisticsName::FragmentShaderInvocations,
                        wgpu::PipelineStatisticsName::VertexShaderInvocations});
    }

    // Fail to create with empty pipeline statistics
    {
        ASSERT_DEVICE_ERROR(CreateQuerySet(deviceWithPipelineStatistics,
                                           wgpu::QueryType::PipelineStatistics, 1, {}));
    }

    // Fail to create with invalid pipeline statistics
    {
        ASSERT_DEVICE_ERROR(
            CreateQuerySet(deviceWithPipelineStatistics, wgpu::QueryType::PipelineStatistics, 1,
                           {static_cast<wgpu::PipelineStatisticsName>(0xFFFFFFFF)}));
    }

    // Fail to create with duplicate pipeline statistics
    {
        ASSERT_DEVICE_ERROR(
            CreateQuerySet(deviceWithPipelineStatistics, wgpu::QueryType::PipelineStatistics, 1,
                           {wgpu::PipelineStatisticsName::VertexShaderInvocations,
                            wgpu::PipelineStatisticsName::VertexShaderInvocations}));
    }
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
