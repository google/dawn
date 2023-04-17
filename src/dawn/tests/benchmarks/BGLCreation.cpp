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

#include <benchmark/benchmark.h>
#include <dawn/webgpu_cpp.h>
#include <array>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/tests/benchmarks/NullDeviceSetup.h"

static void RedundantBGLCreation(benchmark::State& state) {
    static wgpu::Device device = nullptr;

    if (state.thread_index() == 0) {
        std::vector<wgpu::FeatureName> requiredFeatures;
        if (state.threads() > 1) {
            requiredFeatures.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }

        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.requiredFeatures = requiredFeatures.data();
        deviceDesc.requiredFeaturesCount = requiredFeatures.size();
        device = CreateNullDevice(deviceDesc);
    }

    std::vector<wgpu::BindGroupLayoutEntry> entries(state.range(0));
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entries[i].buffer.type = wgpu::BufferBindingType::Uniform;
    }

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = entries.size();
    bglDesc.entries = entries.data();

    thread_local std::vector<wgpu::BindGroupLayout> bgls;
    bgls.reserve(100000);
    for (auto _ : state) {
        bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    }
    bgls.clear();

    if (state.thread_index() == 0) {
        device = nullptr;
    }
}

static void UniqueBGLCreation(benchmark::State& state) {
    static wgpu::Device device = nullptr;

    if (state.thread_index() == 0) {
        std::vector<wgpu::FeatureName> requiredFeatures;
        if (state.threads() > 1) {
            requiredFeatures.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }

        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.requiredFeatures = requiredFeatures.data();
        deviceDesc.requiredFeaturesCount = requiredFeatures.size();
        device = CreateNullDevice(deviceDesc);
    }

    std::vector<wgpu::BindGroupLayoutEntry> entries(state.range(0));
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entries[i].buffer.type = wgpu::BufferBindingType::Uniform;
    }
    entries[0].buffer.minBindingSize = 4u;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = entries.size();
    bglDesc.entries = entries.data();

    thread_local std::vector<wgpu::BindGroupLayout> bgls;
    bgls.reserve(100000);
    for (auto _ : state) {
        entries[0].buffer.minBindingSize += 4;
        bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    }
    bgls.clear();

    if (state.thread_index() == 0) {
        device = nullptr;
    }
}

BENCHMARK(RedundantBGLCreation)
    ->Setup(SetupNullBackend)
    ->Arg(1)
    ->Arg(12)
    ->Threads(1)
    ->Threads(4)
    ->Threads(16);

BENCHMARK(UniqueBGLCreation)
    ->Setup(SetupNullBackend)
    ->Arg(1)
    ->Arg(12)
    ->Threads(1)
    ->Threads(4)
    ->Threads(16);
