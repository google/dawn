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

namespace dawn {
namespace {

// Benchmarks for creation and recreation of objects in Dawn.
class ObjectCreation : public NullDeviceBenchmarkFixture {
  protected:
    ObjectCreation() {
        // Currently, object creation still needs to be implicitly synchronized even though the
        // frontend cache is thread-safe. Once other parts of Dawn are thread-safe, i.e. memory
        // management, these tests should work without synchronization.
        requiredFeatures.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
    }

  private:
    wgpu::DeviceDescriptor GetDeviceDescriptor() const override {
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.requiredFeatures = requiredFeatures.data();
        deviceDesc.requiredFeaturesCount = requiredFeatures.size();
        return deviceDesc;
    }

    std::vector<wgpu::FeatureName> requiredFeatures;
};

BENCHMARK_DEFINE_F(ObjectCreation, SameBindGroupLayout)
(benchmark::State& state) {
    std::vector<wgpu::BindGroupLayoutEntry> entries(state.range(0));
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entries[i].buffer.type = wgpu::BufferBindingType::Uniform;
    }

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = entries.size();
    bglDesc.entries = entries.data();

    std::vector<wgpu::BindGroupLayout> bgls;
    bgls.reserve(100000);
    bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    for (auto _ : state) {
        bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, SameBindGroupLayout)
    ->Arg(1)
    ->Arg(12)
    ->Threads(1)
    ->Threads(4)
    ->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, UniqueBindGroupLayout)
(benchmark::State& state) {
    std::vector<wgpu::BindGroupLayoutEntry> entries(state.range(0));
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entries[i].buffer.type = wgpu::BufferBindingType::Uniform;
        entries[i].buffer.minBindingSize = 4u;
    }

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = entries.size();
    bglDesc.entries = entries.data();

    // Depending on the thread index, we increment a subset of the binding sizes to ensure we create
    // a new unique bind group descriptor. For now, this is just the thread_index if it's smaller
    // than Arg, otherwise its the last index AND the modulo index.
    std::vector<size_t> entryIndices;
    if (state.thread_index() < state.range(0)) {
        entryIndices.push_back(state.thread_index());
    } else {
        entryIndices.push_back(state.thread_index() % state.range(0));
        entryIndices.push_back(state.range(0) - 1);
    }

    std::vector<wgpu::BindGroupLayout> bgls;
    bgls.reserve(100000);
    for (auto _ : state) {
        for (size_t index : entryIndices) {
            entries[index].buffer.minBindingSize += 4;
        }
        bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, UniqueBindGroupLayout)
    ->Arg(12)
    ->Threads(1)
    ->Threads(4)
    ->Threads(16);

}  // namespace
}  // namespace dawn
