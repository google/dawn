// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_REPLAY_REPLAY_H_
#define SRC_DAWN_REPLAY_REPLAY_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/replay/Capture.h"
#include "dawn/replay/Deserialization.h"

namespace dawn::replay {

typedef std::variant<wgpu::Buffer, wgpu::Texture, wgpu::CommandBuffer> Resource;
struct LabeledResource {
    std::string label;
    Resource resource;
};

// Replays a capture. For now we only support replaying the entire capture.
// In the future we'd like to be able to replay up to a certain point.
class Replay {
  public:
    static std::unique_ptr<Replay> Create(wgpu::Device device, const Capture* capture);

    MaybeError Play();

    // Gets the first object of type T with the given label, or nullptr if not
    // found. Note: We don't too much care this is slow as mostly used for
    // testing and debugging.
    template <typename T>
    T GetObjectByLabel(const char* label) const {
        auto isLabel = [label](const LabeledResource& res) { return res.label == label; };
        auto resourcesWithMachingLabel =
            mResources | std::views::values | std::views::filter(isLabel);
        for (const auto& res : resourcesWithMachingLabel) {
            const T* p = std::get_if<T>(&res.resource);
            if (p) {
                return *p;
            }
        }
        return nullptr;
    }

    template <typename T>
    T GetObjectById(schema::ObjectId id) const {
        auto iter = mResources.find(id);
        const T* p = std::get_if<T>(&iter->second.resource);
        return *p;
    }

  private:
    Replay(wgpu::Device device, const Capture* capture);

    MaybeError CreateResource(wgpu::Device device, ReadHead& readHead);

    wgpu::Device mDevice;
    const Capture* mCapture;

    using IdToResourceMap = absl::flat_hash_map<schema::ObjectId, LabeledResource>;
    IdToResourceMap mResources;
};

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_REPLAY_H_
