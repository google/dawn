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

#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/common/Math.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

    // Helper for replacing all occurrences of substr in str with replacement
    std::string ReplaceAll(std::string str,
                           const std::string& substr,
                           const std::string& replacement) {
        size_t pos = 0;
        while ((pos = str.find(substr, pos)) != std::string::npos) {
            str.replace(pos, substr.length(), replacement);
            pos += replacement.length();
        }
        return str;
    }

    // DataMatcherCallback is the callback function by DataMatcher.
    // It is called for each contiguous sequence of bytes that should be checked
    // for equality.
    // offset and size are in units of bytes.
    using DataMatcherCallback = std::function<void(uint32_t offset, uint32_t size)>;

    // DataMatcher is a function pointer to a data matching function.
    // size is the total number of bytes being considered for matching.
    // The callback may be called once or multiple times, and may only consider
    // part of the interval [0, size)
    using DataMatcher = void (*)(uint32_t size, DataMatcherCallback);

    // FullDataMatcher is a DataMatcher that calls callback with the interval
    // [0, size)
    void FullDataMatcher(uint32_t size, DataMatcherCallback callback) {
        callback(0, size);
    }

    // StridedDataMatcher is a DataMatcher that calls callback with the strided
    // intervals of length BYTES_TO_MATCH, skipping BYTES_TO_SKIP.
    // For example: StridedDataMatcher<2, 4>(18, callback) will call callback
    // with the intervals: [0, 2), [6, 8), [12, 14)
    template <int BYTES_TO_MATCH, int BYTES_TO_SKIP>
    void StridedDataMatcher(uint32_t size, DataMatcherCallback callback) {
        uint32_t offset = 0;
        while (offset < size) {
            callback(offset, BYTES_TO_MATCH);
            offset += BYTES_TO_MATCH + BYTES_TO_SKIP;
        }
    }

    // Align returns the WGSL decoration for an explicit structure field alignment
    std::string AlignDeco(uint32_t value) {
        return "@align(" + std::to_string(value) + ") ";
    }

}  // namespace

// Field holds test parameters for ComputeLayoutMemoryBufferTests.Fields
struct Field {
    const char* type;  // Type of the field
    uint32_t align;    // Alignment of the type in bytes
    uint32_t size;     // Natural size of the type in bytes

    uint32_t padded_size = 0;                // Decorated (extended) size of the type in bytes
    DataMatcher matcher = &FullDataMatcher;  // The matching method
    bool storage_buffer_only = false;        // This should only be used for storage buffer tests

    // Sets the padded_size to value.
    // Returns this Field so calls can be chained.
    Field& PaddedSize(uint32_t value) {
        padded_size = value;
        return *this;
    }

    // Sets the matcher to a StridedDataMatcher<BYTES_TO_MATCH, BYTES_TO_SKIP>.
    // Returns this Field so calls can be chained.
    template <int BYTES_TO_MATCH, int BYTES_TO_SKIP>
    Field& Strided() {
        matcher = &StridedDataMatcher<BYTES_TO_MATCH, BYTES_TO_SKIP>;
        return *this;
    }

    // Marks that this should only be used for storage buffer tests.
    // Returns this Field so calls can be chained.
    Field& StorageBufferOnly() {
        storage_buffer_only = true;
        return *this;
    }
};

// StorageClass is an enumerator of storage classes used by ComputeLayoutMemoryBufferTests.Fields
enum class StorageClass {
    Uniform,
    Storage,
};

std::ostream& operator<<(std::ostream& o, StorageClass storageClass) {
    switch (storageClass) {
        case StorageClass::Uniform:
            o << "uniform";
            break;
        case StorageClass::Storage:
            o << "storage";
            break;
    }
    return o;
}

std::ostream& operator<<(std::ostream& o, Field field) {
    o << "@align(" << field.align << ") @size("
      << (field.padded_size > 0 ? field.padded_size : field.size) << ") " << field.type;
    return o;
}

DAWN_TEST_PARAM_STRUCT(ComputeLayoutMemoryBufferTestParams, StorageClass, Field);

class ComputeLayoutMemoryBufferTests
    : public DawnTestWithParams<ComputeLayoutMemoryBufferTestParams> {
    void SetUp() override {
        DawnTestBase::SetUp();
    }
};

TEST_P(ComputeLayoutMemoryBufferTests, Fields) {
    // Sentinel value markers codes used to check that the start and end of
    // structures are correctly aligned. Each of these codes are distinct and
    // are not likely to be confused with data.
    constexpr uint32_t kDataHeaderCode = 0xa0b0c0a0u;
    constexpr uint32_t kDataFooterCode = 0x40302010u;
    constexpr uint32_t kInputHeaderCode = 0x91827364u;
    constexpr uint32_t kInputFooterCode = 0x19283764u;

    // Byte codes used for field padding. The MSB is set for each of these.
    // The field data has the MSB 0.
    constexpr uint8_t kDataAlignPaddingCode = 0xfeu;
    constexpr uint8_t kFieldAlignPaddingCode = 0xfdu;
    constexpr uint8_t kFieldSizePaddingCode = 0xdcu;
    constexpr uint8_t kDataSizePaddingCode = 0xdbu;
    constexpr uint8_t kInputFooterAlignPaddingCode = 0xdau;
    constexpr uint8_t kInputTailPaddingCode = 0xd9u;

    // Status codes returned by the shader.
    constexpr uint32_t kStatusBadInputHeader = 100u;
    constexpr uint32_t kStatusBadInputFooter = 101u;
    constexpr uint32_t kStatusBadDataHeader = 102u;
    constexpr uint32_t kStatusBadDataFooter = 103u;
    constexpr uint32_t kStatusOk = 200u;

    const Field& field = GetParam().mField;

    const bool isUniform = GetParam().mStorageClass == StorageClass::Uniform;

    std::string shader = R"(
struct Data {
    header : u32,
    @align({field_align}) @size({field_size}) field : {field_type},
    footer : u32,
}

struct Input {
    header : u32,
    {data_align}data : Data,
    {footer_align}footer : u32,
}

struct Output {
    data : {field_type}
}

struct Status {
    code : u32
}

@group(0) @binding(0) var<{input_qualifiers}> input : Input;
@group(0) @binding(1) var<storage, read_write> output : Output;
@group(0) @binding(2) var<storage, read_write> status : Status;

@stage(compute) @workgroup_size(1,1,1)
fn main() {
    if (input.header != {input_header_code}u) {
        status.code = {status_bad_input_header}u;
    } else if (input.footer != {input_footer_code}u) {
        status.code = {status_bad_input_footer}u;
    } else if (input.data.header != {data_header_code}u) {
        status.code = {status_bad_data_header}u;
    } else if (input.data.footer != {data_footer_code}u) {
        status.code = {status_bad_data_footer}u;
    } else {
        status.code = {status_ok}u;
        output.data = input.data.field;
    }
})";

    // https://www.w3.org/TR/WGSL/#alignment-and-size
    // Structure size: roundUp(AlignOf(S), OffsetOf(S, L) + SizeOf(S, L))
    // https://www.w3.org/TR/WGSL/#storage-class-constraints
    // RequiredAlignOf(S, uniform): roundUp(16, max(AlignOf(T0), ..., AlignOf(TN)))
    uint32_t dataAlign = isUniform ? std::max(16u, field.align) : field.align;

    // https://www.w3.org/TR/WGSL/#structure-layout-rules
    // Note: When underlying the target is a Vulkan device, we assume the device does not support
    // the scalarBlockLayout feature. Therefore, a data value must not be placed in the padding at
    // the end of a structure or matrix, nor in the padding at the last element of an array.
    uint32_t footerAlign = isUniform ? 16 : 4;

    shader = ReplaceAll(shader, "{data_align}", isUniform ? AlignDeco(dataAlign) : "");
    shader = ReplaceAll(shader, "{field_align}", std::to_string(field.align));
    shader = ReplaceAll(shader, "{footer_align}", isUniform ? AlignDeco(footerAlign) : "");
    shader = ReplaceAll(shader, "{field_size}",
                        std::to_string(field.padded_size > 0 ? field.padded_size : field.size));
    shader = ReplaceAll(shader, "{field_type}", field.type);
    shader = ReplaceAll(shader, "{input_header_code}", std::to_string(kInputHeaderCode));
    shader = ReplaceAll(shader, "{input_footer_code}", std::to_string(kInputFooterCode));
    shader = ReplaceAll(shader, "{data_header_code}", std::to_string(kDataHeaderCode));
    shader = ReplaceAll(shader, "{data_footer_code}", std::to_string(kDataFooterCode));
    shader = ReplaceAll(shader, "{status_bad_input_header}", std::to_string(kStatusBadInputHeader));
    shader = ReplaceAll(shader, "{status_bad_input_footer}", std::to_string(kStatusBadInputFooter));
    shader = ReplaceAll(shader, "{status_bad_data_header}", std::to_string(kStatusBadDataHeader));
    shader = ReplaceAll(shader, "{status_bad_data_footer}", std::to_string(kStatusBadDataFooter));
    shader = ReplaceAll(shader, "{status_ok}", std::to_string(kStatusOk));
    shader = ReplaceAll(shader, "{input_qualifiers}",
                        isUniform ? "uniform"  //
                                  : "storage, read_write");

    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader.c_str());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Build the input and expected data.
    std::vector<uint8_t> inputData;     // The whole SSBO data
    std::vector<uint8_t> expectedData;  // The expected data to be copied by the shader
    {
        auto PushU32 = [&inputData](uint32_t u32) {
            inputData.emplace_back((u32 >> 0) & 0xff);
            inputData.emplace_back((u32 >> 8) & 0xff);
            inputData.emplace_back((u32 >> 16) & 0xff);
            inputData.emplace_back((u32 >> 24) & 0xff);
        };
        auto AlignTo = [&inputData](uint32_t alignment, uint8_t code) {
            uint32_t target = Align(inputData.size(), alignment);
            uint32_t bytes = target - inputData.size();
            for (uint32_t i = 0; i < bytes; i++) {
                inputData.emplace_back(code);
            }
        };
        PushU32(kInputHeaderCode);                  // Input.header
        AlignTo(dataAlign, kDataAlignPaddingCode);  // Input.data
        {
            PushU32(kDataHeaderCode);                      // Input.data.header
            AlignTo(field.align, kFieldAlignPaddingCode);  // Input.data.field
            for (uint32_t i = 0; i < field.size; i++) {
                // The data has the MSB cleared to distinguish it from the
                // padding codes.
                uint8_t code = i & 0x7f;
                inputData.emplace_back(code);  // Input.data.field
                expectedData.emplace_back(code);
            }
            for (uint32_t i = field.size; i < field.padded_size; i++) {
                inputData.emplace_back(kFieldSizePaddingCode);  // Input.data.field padding
            }
            PushU32(kDataFooterCode);                    // Input.data.footer
            AlignTo(field.align, kDataSizePaddingCode);  // Input.data padding
        }
        AlignTo(footerAlign, kInputFooterAlignPaddingCode);  // Input.footer @align
        PushU32(kInputFooterCode);                           // Input.footer
        AlignTo(256, kInputTailPaddingCode);                 // Input padding
    }

    // Set up input storage buffer
    wgpu::Buffer inputBuf = utils::CreateBufferFromData(
        device, inputData.data(), inputData.size(),
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
            (isUniform ? wgpu::BufferUsage::Uniform : wgpu::BufferUsage::Storage));

    // Set up output storage buffer
    wgpu::BufferDescriptor outputDesc;
    outputDesc.size = field.size;
    outputDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer outputBuf = device.CreateBuffer(&outputDesc);

    // Set up status storage buffer
    wgpu::BufferDescriptor statusDesc;
    statusDesc.size = 4u;
    statusDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer statusBuf = device.CreateBuffer(&statusDesc);

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, inputBuf},
                                                         {1, outputBuf},
                                                         {2, statusBuf},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    // Check the status
    EXPECT_BUFFER_U32_EQ(kStatusOk, statusBuf, 0) << "status code error" << std::endl
                                                  << "Shader: " << shader;

    // Check the data
    field.matcher(field.size, [&](uint32_t offset, uint32_t size) {
        EXPECT_BUFFER_U8_RANGE_EQ(expectedData.data() + offset, outputBuf, offset, size)
            << "offset: " << offset;
    });
}

namespace {

    auto GenerateParams() {
        auto params = MakeParamGenerator<ComputeLayoutMemoryBufferTestParams>(
            {
                D3D12Backend(), MetalBackend(), VulkanBackend(),
                // TODO(crbug.com/dawn/942)
                // There was a compiler error: Buffer block cannot be expressed as any of std430,
                // std140, scalar, even with enhanced layouts. You can try flattening this block to
                // support a more flexible layout.
                // OpenGLBackend(),
                // OpenGLESBackend(),
            },
            {StorageClass::Storage, StorageClass::Uniform},
            {
                // See https://www.w3.org/TR/WGSL/#alignment-and-size
                // Scalar types with no custom alignment or size
                Field{"i32", /* align */ 4, /* size */ 4},
                Field{"u32", /* align */ 4, /* size */ 4},
                Field{"f32", /* align */ 4, /* size */ 4},

                // Scalar types with custom alignment
                Field{"i32", /* align */ 16, /* size */ 4},
                Field{"u32", /* align */ 16, /* size */ 4},
                Field{"f32", /* align */ 16, /* size */ 4},

                // Scalar types with custom size
                Field{"i32", /* align */ 4, /* size */ 4}.PaddedSize(24),
                Field{"u32", /* align */ 4, /* size */ 4}.PaddedSize(24),
                Field{"f32", /* align */ 4, /* size */ 4}.PaddedSize(24),

                // Vector types with no custom alignment or size
                Field{"vec2<i32>", /* align */ 8, /* size */ 8},
                Field{"vec2<u32>", /* align */ 8, /* size */ 8},
                Field{"vec2<f32>", /* align */ 8, /* size */ 8},
                Field{"vec3<i32>", /* align */ 16, /* size */ 12},
                Field{"vec3<u32>", /* align */ 16, /* size */ 12},
                Field{"vec3<f32>", /* align */ 16, /* size */ 12},
                Field{"vec4<i32>", /* align */ 16, /* size */ 16},
                Field{"vec4<u32>", /* align */ 16, /* size */ 16},
                Field{"vec4<f32>", /* align */ 16, /* size */ 16},

                // Vector types with custom alignment
                Field{"vec2<i32>", /* align */ 32, /* size */ 8},
                Field{"vec2<u32>", /* align */ 32, /* size */ 8},
                Field{"vec2<f32>", /* align */ 32, /* size */ 8},
                Field{"vec3<i32>", /* align */ 32, /* size */ 12},
                Field{"vec3<u32>", /* align */ 32, /* size */ 12},
                Field{"vec3<f32>", /* align */ 32, /* size */ 12},
                Field{"vec4<i32>", /* align */ 32, /* size */ 16},
                Field{"vec4<u32>", /* align */ 32, /* size */ 16},
                Field{"vec4<f32>", /* align */ 32, /* size */ 16},

                // Vector types with custom size
                Field{"vec2<i32>", /* align */ 8, /* size */ 8}.PaddedSize(24),
                Field{"vec2<u32>", /* align */ 8, /* size */ 8}.PaddedSize(24),
                Field{"vec2<f32>", /* align */ 8, /* size */ 8}.PaddedSize(24),
                Field{"vec3<i32>", /* align */ 16, /* size */ 12}.PaddedSize(24),
                Field{"vec3<u32>", /* align */ 16, /* size */ 12}.PaddedSize(24),
                Field{"vec3<f32>", /* align */ 16, /* size */ 12}.PaddedSize(24),
                Field{"vec4<i32>", /* align */ 16, /* size */ 16}.PaddedSize(24),
                Field{"vec4<u32>", /* align */ 16, /* size */ 16}.PaddedSize(24),
                Field{"vec4<f32>", /* align */ 16, /* size */ 16}.PaddedSize(24),

                // Matrix types with no custom alignment or size
                Field{"mat2x2<f32>", /* align */ 8, /* size */ 16},
                Field{"mat3x2<f32>", /* align */ 8, /* size */ 24},
                Field{"mat4x2<f32>", /* align */ 8, /* size */ 32},
                Field{"mat2x3<f32>", /* align */ 16, /* size */ 32}.Strided<12, 4>(),
                Field{"mat3x3<f32>", /* align */ 16, /* size */ 48}.Strided<12, 4>(),
                Field{"mat4x3<f32>", /* align */ 16, /* size */ 64}.Strided<12, 4>(),
                Field{"mat2x4<f32>", /* align */ 16, /* size */ 32},
                Field{"mat3x4<f32>", /* align */ 16, /* size */ 48},
                Field{"mat4x4<f32>", /* align */ 16, /* size */ 64},

                // Matrix types with custom alignment
                Field{"mat2x2<f32>", /* align */ 32, /* size */ 16},
                Field{"mat3x2<f32>", /* align */ 32, /* size */ 24},
                Field{"mat4x2<f32>", /* align */ 32, /* size */ 32},
                Field{"mat2x3<f32>", /* align */ 32, /* size */ 32}.Strided<12, 4>(),
                Field{"mat3x3<f32>", /* align */ 32, /* size */ 48}.Strided<12, 4>(),
                Field{"mat4x3<f32>", /* align */ 32, /* size */ 64}.Strided<12, 4>(),
                Field{"mat2x4<f32>", /* align */ 32, /* size */ 32},
                Field{"mat3x4<f32>", /* align */ 32, /* size */ 48},
                Field{"mat4x4<f32>", /* align */ 32, /* size */ 64},

                // Matrix types with custom size
                Field{"mat2x2<f32>", /* align */ 8, /* size */ 16}.PaddedSize(128),
                Field{"mat3x2<f32>", /* align */ 8, /* size */ 24}.PaddedSize(128),
                Field{"mat4x2<f32>", /* align */ 8, /* size */ 32}.PaddedSize(128),
                Field{"mat2x3<f32>", /* align */ 16, /* size */ 32}
                    .PaddedSize(128)
                    .Strided<12, 4>(),
                Field{"mat3x3<f32>", /* align */ 16, /* size */ 48}
                    .PaddedSize(128)
                    .Strided<12, 4>(),
                Field{"mat4x3<f32>", /* align */ 16, /* size */ 64}
                    .PaddedSize(128)
                    .Strided<12, 4>(),
                Field{"mat2x4<f32>", /* align */ 16, /* size */ 32}.PaddedSize(128),
                Field{"mat3x4<f32>", /* align */ 16, /* size */ 48}.PaddedSize(128),
                Field{"mat4x4<f32>", /* align */ 16, /* size */ 64}.PaddedSize(128),

                // Array types with no custom alignment or size.
                // Note: The use of StorageBufferOnly() is due to UBOs requiring 16 byte alignment
                // of array elements. See https://www.w3.org/TR/WGSL/#storage-class-constraints
                Field{"array<u32, 1>", /* align */ 4, /* size */ 4}.StorageBufferOnly(),
                Field{"array<u32, 2>", /* align */ 4, /* size */ 8}.StorageBufferOnly(),
                Field{"array<u32, 3>", /* align */ 4, /* size */ 12}.StorageBufferOnly(),
                Field{"array<u32, 4>", /* align */ 4, /* size */ 16}.StorageBufferOnly(),
                Field{"array<vec4<u32>, 1>", /* align */ 16, /* size */ 16},
                Field{"array<vec4<u32>, 2>", /* align */ 16, /* size */ 32},
                Field{"array<vec4<u32>, 3>", /* align */ 16, /* size */ 48},
                Field{"array<vec4<u32>, 4>", /* align */ 16, /* size */ 64},
                Field{"array<vec3<u32>, 4>", /* align */ 16, /* size */ 64}.Strided<12, 4>(),

                // Array types with custom alignment
                Field{"array<u32, 1>", /* align */ 32, /* size */ 4}.StorageBufferOnly(),
                Field{"array<u32, 2>", /* align */ 32, /* size */ 8}.StorageBufferOnly(),
                Field{"array<u32, 3>", /* align */ 32, /* size */ 12}.StorageBufferOnly(),
                Field{"array<u32, 4>", /* align */ 32, /* size */ 16}.StorageBufferOnly(),
                Field{"array<vec4<u32>, 1>", /* align */ 32, /* size */ 16},
                Field{"array<vec4<u32>, 2>", /* align */ 32, /* size */ 32},
                Field{"array<vec4<u32>, 3>", /* align */ 32, /* size */ 48},
                Field{"array<vec4<u32>, 4>", /* align */ 32, /* size */ 64},
                Field{"array<vec3<u32>, 4>", /* align */ 32, /* size */ 64}.Strided<12, 4>(),

                // Array types with custom size
                Field{"array<u32, 1>", /* align */ 4, /* size */ 4}
                    .PaddedSize(128)
                    .StorageBufferOnly(),
                Field{"array<u32, 2>", /* align */ 4, /* size */ 8}
                    .PaddedSize(128)
                    .StorageBufferOnly(),
                Field{"array<u32, 3>", /* align */ 4, /* size */ 12}
                    .PaddedSize(128)
                    .StorageBufferOnly(),
                Field{"array<u32, 4>", /* align */ 4, /* size */ 16}
                    .PaddedSize(128)
                    .StorageBufferOnly(),
                Field{"array<vec3<u32>, 4>", /* align */ 16, /* size */ 64}
                    .PaddedSize(128)
                    .Strided<12, 4>(),
            });

        std::vector<ComputeLayoutMemoryBufferTestParams> filtered;
        for (auto param : params) {
            if (param.mStorageClass != StorageClass::Storage && param.mField.storage_buffer_only) {
                continue;
            }
            filtered.emplace_back(param);
        }
        return filtered;
    }

    INSTANTIATE_TEST_SUITE_P(
        ,
        ComputeLayoutMemoryBufferTests,
        ::testing::ValuesIn(GenerateParams()),
        DawnTestBase::PrintToStringParamName("ComputeLayoutMemoryBufferTests"));
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ComputeLayoutMemoryBufferTests);

}  // namespace
