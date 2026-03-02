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

#include "dawn/replay/Replay.h"

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Constants.h"
#include "dawn/replay/BlitBufferToDepthTexture.h"
#include "dawn/replay/Capture.h"
#include "dawn/replay/Deserialization.h"
#include "src/dawn/replay/ReplayImpl.h"

namespace dawn::replay {

namespace schema {
#define DAWN_REPLAY_ENUM_TO_STRING_MEMBER(NAME, ...) \
    case EnumType::NAME:                             \
        return #NAME;

#define DAWN_REPLAY_ENUM_TO_STRING(NAME, MEMBERS)                    \
    const char* EnumToString(schema::NAME value) {                   \
        using EnumType = NAME;                                       \
        switch (value) {                                             \
            MEMBERS(DAWN_REPLAY_ENUM_TO_STRING_MEMBER)               \
            default:                                                 \
                return "UNKNOWN";                                    \
        }                                                            \
    }                                                                \
    std::ostream& operator<<(std::ostream& os, schema::NAME value) { \
        return os << EnumToString(value);                            \
    }

DAWN_REPLAY_OBJECT_TYPES_ENUM(DAWN_REPLAY_ENUM_TO_STRING)
DAWN_REPLAY_COMMAND_BUFFER_COMMANDS_ENUM(DAWN_REPLAY_ENUM_TO_STRING)
DAWN_REPLAY_ROOT_COMMANDS_ENUM(DAWN_REPLAY_ENUM_TO_STRING)

#undef DAWN_REPLAY_ENUM_TO_STRING_MEMBER
#undef DAWN_REPLAY_ENUM_TO_STRING

}  // namespace schema

Replay::~Replay() = default;

std::unique_ptr<Replay> Replay::Create(wgpu::Device device, std::unique_ptr<Capture> capture) {
    return ReplayImpl::Create(device, std::move(capture));
}

template <typename T>
T Replay::GetObjectByLabel(std::string_view label) const {
    if (auto* impl = static_cast<const ReplayImpl*>(this)) {
        return impl->GetObjectByLabel<T>(label);
    }
    return nullptr;
}

bool Replay::Play() {
    if (auto* impl = static_cast<ReplayImpl*>(this)) {
        auto result = impl->Play();
        return result.IsSuccess();
    }
    return false;
}

template wgpu::BindGroup Replay::GetObjectByLabel<wgpu::BindGroup>(std::string_view label) const;
template wgpu::BindGroupLayout Replay::GetObjectByLabel<wgpu::BindGroupLayout>(
    std::string_view label) const;
template wgpu::Buffer Replay::GetObjectByLabel<wgpu::Buffer>(std::string_view label) const;
template wgpu::CommandBuffer Replay::GetObjectByLabel<wgpu::CommandBuffer>(
    std::string_view label) const;
template wgpu::ComputePipeline Replay::GetObjectByLabel<wgpu::ComputePipeline>(
    std::string_view label) const;
template wgpu::Device Replay::GetObjectByLabel<wgpu::Device>(std::string_view label) const;
template wgpu::PipelineLayout Replay::GetObjectByLabel<wgpu::PipelineLayout>(
    std::string_view label) const;
template wgpu::QuerySet Replay::GetObjectByLabel<wgpu::QuerySet>(std::string_view label) const;
template wgpu::RenderBundle Replay::GetObjectByLabel<wgpu::RenderBundle>(
    std::string_view label) const;
template wgpu::RenderPipeline Replay::GetObjectByLabel<wgpu::RenderPipeline>(
    std::string_view label) const;
template wgpu::Sampler Replay::GetObjectByLabel<wgpu::Sampler>(std::string_view label) const;
template wgpu::ShaderModule Replay::GetObjectByLabel<wgpu::ShaderModule>(
    std::string_view label) const;
template wgpu::Texture Replay::GetObjectByLabel<wgpu::Texture>(std::string_view label) const;
template wgpu::ExternalTexture Replay::GetObjectByLabel<wgpu::ExternalTexture>(
    std::string_view label) const;
template wgpu::TexelBufferView Replay::GetObjectByLabel<wgpu::TexelBufferView>(
    std::string_view label) const;
template wgpu::TextureView Replay::GetObjectByLabel<wgpu::TextureView>(
    std::string_view label) const;

typedef std::variant<wgpu::BindGroup,
                     wgpu::BindGroupLayout,
                     wgpu::Buffer,
                     wgpu::CommandBuffer,
                     wgpu::ComputePipeline,
                     wgpu::Device,
                     wgpu::ExternalTexture,
                     wgpu::PipelineLayout,
                     wgpu::QuerySet,
                     wgpu::RenderBundle,
                     wgpu::RenderPipeline,
                     wgpu::Sampler,
                     wgpu::ShaderModule,
                     wgpu::TexelBufferView,
                     wgpu::Texture,
                     wgpu::TextureView>
    Resource;

struct LabeledResource {
    std::string label;
    Resource resource;
};

class DawnResourceVisitor : public ResourceVisitor {
  public:
    using ResourceVisitor::operator();
    explicit DawnResourceVisitor(DawnRootCommandVisitor* visitor) : mVisitor(visitor) {}

    MaybeError operator()(const BindGroupData& data) override;
    MaybeError operator()(const BindGroupLayoutData& data) override;
    MaybeError operator()(const schema::Buffer& data) override;
    MaybeError operator()(const CommandBufferData& data) override;
    MaybeError operator()(const schema::ComputePipeline& data) override;
    MaybeError operator()(const schema::ExternalTexture& data) override;
    MaybeError operator()(const schema::PipelineLayout& data) override;
    MaybeError operator()(const schema::QuerySet& data) override;
    MaybeError operator()(const RenderBundleData& data) override;
    MaybeError operator()(const schema::RenderPipeline& data) override;
    MaybeError operator()(const schema::Sampler& data) override;
    MaybeError operator()(const schema::ShaderModule& data) override;
    MaybeError operator()(const schema::TexelBufferView& data) override;
    MaybeError operator()(const schema::Texture& data) override;
    MaybeError operator()(const schema::TextureView& data) override;

  private:
    template <typename T>
    MaybeError Create(const T& data);

    DawnRootCommandVisitor* mVisitor;
};

class DawnRootCommandVisitor : public RootCommandVisitor {
  public:
    using RootCommandVisitor::operator();
    explicit DawnRootCommandVisitor(wgpu::Device device)
        : mDevice(device), mResourceVisitor(this) {}

    MaybeError operator()(const CreateResourceData& data) override {
        mCurrentResourceId = data.resource.id;
        mCurrentResourceLabel = data.resource.label;
        return std::visit(mResourceVisitor, data.data);
    }

    MaybeError operator()(const schema::RootCommandWriteBufferCmdData& data) override;
    MaybeError operator()(const schema::RootCommandWriteTextureCmdData& data) override;
    MaybeError operator()(const schema::RootCommandQueueSubmitCmdData& data) override;
    MaybeError operator()(const schema::RootCommandSetLabelCmdData& data) override;
    MaybeError operator()(const schema::RootCommandInitTextureCmdData& data) override;
    MaybeError operator()(const schema::RootCommandUnmapBufferCmdData& data) override { return {}; }
    MaybeError operator()(const schema::RootCommandEndCmdData& data) override { return {}; }

    ResourceVisitor& GetResourceVisitor() override { return mResourceVisitor; }

    wgpu::Device GetDevice() const { return mDevice; }
    schema::ObjectId GetCurrentResourceId() const { return mCurrentResourceId; }
    const std::string& GetCurrentResourceLabel() const { return mCurrentResourceLabel; }

    template <typename T>
    T GetObjectById(schema::ObjectId id) const {
        if (id == 0) {
            return nullptr;
        }
        auto iter = mResources.find(id);
        if (iter == mResources.end()) {
            return nullptr;
        }
        const T* p = std::get_if<T>(&iter->second.resource);
        return p ? *p : nullptr;
    }

    template <typename T>
    T GetObjectByLabel(std::string_view label) const {
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
    const std::string& GetLabel(const T& object) const {
        if (object == nullptr) {
            return kNotFound;
        }
        return GetLabel(GetObjectId(object));
    }

    const std::string& GetLabel(schema::ObjectId id) const {
        auto iter = mResources.find(id);
        if (iter == mResources.end()) {
            return kNotFound;
        }
        return iter->second.label;
    }

    template <typename T>
    schema::ObjectId GetObjectId(const T& object) const {
        auto iter = mResourceToIdMap.find(object.Get());
        return iter != mResourceToIdMap.end() ? iter->second : 0;
    }

    template <typename T>
    void AddResource(schema::ObjectId id, const std::string& label, T resource) {
        mResources.emplace(id, LabeledResource{label, resource});
        mResourceToIdMap.emplace(resource.Get(), id);
    }

    void AddResource(schema::ObjectId id, const std::string& label, Resource resource) {
        std::visit(
            [&](auto& r) {
                if constexpr (!std::is_same_v<std::decay_t<decltype(r)>, std::monostate>) {
                    mResources.emplace(id, LabeledResource{label, r});
                    mResourceToIdMap.emplace(r.Get(), id);
                }
            },
            resource);
    }

    MaybeError SetLabel(schema::ObjectId id, schema::ObjectType type, const std::string& label);

    BlitBufferToDepthTexture& GetBlitBufferToDepthTexture() { return mBlitBufferToDepthTexture; }

    void SetContentReadHead(ReadHead* readHead) override { mContentReadHead = readHead; }

  private:
    wgpu::Device mDevice;
    DawnResourceVisitor mResourceVisitor;

    using IdToResourceMap = absl::flat_hash_map<schema::ObjectId, LabeledResource>;
    IdToResourceMap mResources;

    using ResourcePtrToIdMap = absl::flat_hash_map<const void*, schema::ObjectId>;
    ResourcePtrToIdMap mResourceToIdMap;

    BlitBufferToDepthTexture mBlitBufferToDepthTexture;

    schema::ObjectId mCurrentResourceId;
    std::string mCurrentResourceLabel;

    ReadHead* mContentReadHead = nullptr;

    std::string kNotFound;
};

namespace {

bool IsSwizzleIdentity(const wgpu::TextureComponentSwizzle& swizzle) {
    return (swizzle.r == wgpu::ComponentSwizzle::R ||
            swizzle.r == wgpu::ComponentSwizzle::Undefined) &&
           (swizzle.g == wgpu::ComponentSwizzle::G ||
            swizzle.g == wgpu::ComponentSwizzle::Undefined) &&
           (swizzle.b == wgpu::ComponentSwizzle::B ||
            swizzle.b == wgpu::ComponentSwizzle::Undefined) &&
           (swizzle.a == wgpu::ComponentSwizzle::A ||
            swizzle.a == wgpu::ComponentSwizzle::Undefined);
}

wgpu::Origin3D ToWGPU(const schema::Origin3D& origin) {
    return wgpu::Origin3D{
        .x = origin.x,
        .y = origin.y,
        .z = origin.z,
    };
}

wgpu::Origin2D ToWGPU(const schema::Origin2D& origin) {
    return wgpu::Origin2D{
        .x = origin.x,
        .y = origin.y,
    };
}

wgpu::Extent3D ToWGPU(const schema::Extent3D& extent) {
    return wgpu::Extent3D{
        .width = extent.width,
        .height = extent.height,
        .depthOrArrayLayers = extent.depthOrArrayLayers,
    };
}

wgpu::Extent2D ToWGPU(const schema::Extent2D& extent) {
    return wgpu::Extent2D{
        .width = extent.width,
        .height = extent.height,
    };
}

wgpu::Color ToWGPU(const schema::Color& color) {
    return wgpu::Color{
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
}

wgpu::PassTimestampWrites ToWGPU(const DawnRootCommandVisitor& replay,
                                 const schema::TimestampWrites& writes) {
    return wgpu::PassTimestampWrites{
        .nextInChain = nullptr,
        .querySet = replay.GetObjectById<wgpu::QuerySet>(writes.querySetId),
        .beginningOfPassWriteIndex = writes.beginningOfPassWriteIndex,
        .endOfPassWriteIndex = writes.endOfPassWriteIndex,
    };
}

std::vector<wgpu::ConstantEntry> ToWGPU(const std::vector<schema::PipelineConstant>& constants) {
    std::vector<wgpu::ConstantEntry> entries;
    entries.reserve(constants.size());
    std::transform(constants.begin(), constants.end(), std::back_inserter(entries),
                   [](const auto& constant) {
                       return wgpu::ConstantEntry{
                           .key = wgpu::StringView(constant.name),
                           .value = constant.value,
                       };
                   });
    return entries;
}

wgpu::TexelCopyBufferLayout ToWGPU(const schema::TexelCopyBufferLayout& info) {
    return wgpu::TexelCopyBufferLayout{
        .offset = info.offset,
        .bytesPerRow = info.bytesPerRow,
        .rowsPerImage = info.rowsPerImage,
    };
}

wgpu::TexelCopyBufferInfo ToWGPU(const DawnRootCommandVisitor& replay,
                                 const schema::TexelCopyBufferInfo& info) {
    return wgpu::TexelCopyBufferInfo{
        .layout = ToWGPU(info.layout),
        .buffer = replay.GetObjectById<wgpu::Buffer>(info.bufferId),
    };
}

wgpu::TexelCopyTextureInfo ToWGPU(const DawnRootCommandVisitor& replay,
                                  const schema::TexelCopyTextureInfo& info) {
    return wgpu::TexelCopyTextureInfo{
        .texture = replay.GetObjectById<wgpu::Texture>(info.textureId),
        .mipLevel = info.mipLevel,
        .origin = ToWGPU(info.origin),
        .aspect = static_cast<wgpu::TextureAspect>(info.aspect),
    };
}

wgpu::StencilFaceState ToWGPU(const schema::StencilFaceState& state) {
    return wgpu::StencilFaceState{
        .compare = state.compare,
        .failOp = state.failOp,
        .depthFailOp = state.depthFailOp,
        .passOp = state.passOp,
    };
}

wgpu::BlendComponent ToWGPU(const schema::BlendComponent& component) {
    return wgpu::BlendComponent{
        .operation = component.operation,
        .srcFactor = component.srcFactor,
        .dstFactor = component.dstFactor,
    };
}

wgpu::BlendState ToWGPU(const schema::BlendState& state) {
    return wgpu::BlendState{
        .color = ToWGPU(state.color),
        .alpha = ToWGPU(state.alpha),
    };
}

bool IsBlendComponentEnabled(const wgpu::BlendComponent& component) {
    return component.operation != wgpu::BlendOperation::Add ||
           component.srcFactor != wgpu::BlendFactor::One ||
           component.dstFactor != wgpu::BlendFactor::Zero;
}

bool IsBlendEnabled(const wgpu::BlendState& blend) {
    return IsBlendComponentEnabled(blend.color) || IsBlendComponentEnabled(blend.alpha);
}

MaybeError ReadContentIntoBuffer(ReadHead& readHead,
                                 wgpu::Device device,
                                 wgpu::Buffer buffer,
                                 uint64_t bufferOffset,
                                 uint64_t size) {
    const uint32_t* data;
    DAWN_TRY_ASSIGN(data, readHead.GetData(size));

    device.GetQueue().WriteBuffer(buffer, bufferOffset, data, size);
    return {};
}

MaybeError ReadContentIntoTexture(const DawnRootCommandVisitor& replay,
                                  ReadHead& readHead,
                                  wgpu::Device device,
                                  const schema::RootCommandWriteTextureCmdData& cmdData) {
    const uint64_t dataSize = (cmdData.dataSize + 3) & ~3;

    const uint32_t* data;
    DAWN_TRY_ASSIGN(data, readHead.GetData(dataSize));

    wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, cmdData.destination);
    wgpu::TexelCopyBufferLayout layout = ToWGPU(cmdData.layout);
    wgpu::Extent3D size = ToWGPU(cmdData.size);
    device.GetQueue().WriteTexture(&dst, data, cmdData.dataSize, &layout, &size);
    return {};
}

bool TextureFormatNeedsBlit(wgpu::TextureFormat format, wgpu::TextureAspect aspect) {
    switch (format) {
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
            return true;
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8: {
            return aspect == wgpu::TextureAspect::DepthOnly;
        }
        default:
            return false;
    }
}

MaybeError InitializeTexture(const DawnRootCommandVisitor& replay,
                             BlitBufferToDepthTexture& blitBufferToDepthTexture,
                             ReadHead& readHead,
                             wgpu::Device device,
                             const schema::RootCommandInitTextureCmdData& cmdData) {
    const uint64_t dataSize = (cmdData.dataSize + 3) & ~3;

    const uint32_t* data;
    DAWN_TRY_ASSIGN(data, readHead.GetData(dataSize));

    wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, cmdData.destination);
    wgpu::TexelCopyBufferLayout layout = ToWGPU(cmdData.layout);
    wgpu::Extent3D size = ToWGPU(cmdData.size);

    if (TextureFormatNeedsBlit(dst.texture.GetFormat(), dst.aspect)) {
        DAWN_TRY(blitBufferToDepthTexture.Blit(device, dst, data, cmdData.dataSize, layout, size));
    } else {
        device.GetQueue().WriteTexture(&dst, data, cmdData.dataSize, &layout, &size);
    }
    return {};
}

struct BindGroupEntryVisitor {
    const DawnRootCommandVisitor& replay;
    std::vector<wgpu::BindGroupEntry>& entries;
    std::vector<std::unique_ptr<wgpu::ExternalTextureBindingEntry>>& externalTextureBindingEntries;

    template <typename T>
    MaybeError operator()(const T& data) {
        wgpu::BindGroupEntry entry;
        entry.binding = data.binding;
        DAWN_TRY(FillEntry(entry, data.data));
        entries.push_back(entry);
        return {};
    }

    MaybeError operator()(const std::monostate&) {
        return DAWN_INTERNAL_ERROR("invalid bind group entry");
    }

  private:
    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeBufferBindingData& data) {
        entry.buffer = replay.GetObjectById<wgpu::Buffer>(data.bufferId);
        entry.offset = data.offset;
        entry.size = data.size;
        return {};
    }

    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeSamplerBindingData& data) {
        entry.sampler = replay.GetObjectById<wgpu::Sampler>(data.samplerId);
        return {};
    }

    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeTextureBindingData& data) {
        entry.textureView = replay.GetObjectById<wgpu::TextureView>(data.textureViewId);
        return {};
    }

    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeStorageTextureBindingData& data) {
        entry.textureView = replay.GetObjectById<wgpu::TextureView>(data.textureViewId);
        return {};
    }

    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeTexelBufferBindingData& data) {
        return DAWN_INTERNAL_ERROR("texel buffer binding not supported");
    }

    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeInputAttachmentBindingInfoData& data) {
        entry.textureView = replay.GetObjectById<wgpu::TextureView>(data.textureViewId);
        return {};
    }

    MaybeError FillEntry(wgpu::BindGroupEntry& entry,
                         const schema::BindGroupEntryTypeExternalTextureBindingData& data) {
        if (data.externalTextureId != 0) {
            auto& externalBindingEntryPtr = externalTextureBindingEntries.emplace_back(
                std::make_unique<wgpu::ExternalTextureBindingEntry>());
            externalBindingEntryPtr->externalTexture =
                replay.GetObjectById<wgpu::ExternalTexture>(data.externalTextureId);
            entry.nextInChain = externalBindingEntryPtr.get();
        } else {
            // External texture binding can bind a regular texture view
            DAWN_ASSERT(data.textureViewId != 0);
            entry.textureView = replay.GetObjectById<wgpu::TextureView>(data.textureViewId);
        }
        return {};
    }

    template <typename T>
    MaybeError FillEntry(wgpu::BindGroupEntry& entry, const T&) {
        return {};
    }
};

struct BindGroupLayoutEntryVisitor {
    wgpu::BindGroupLayoutEntry& entry;
    wgpu::ExternalTextureBindingLayout& externalTextureBindingLayout;

    template <typename T>
    MaybeError operator()(const T& data) {
        entry.binding = data.binding.binding;
        entry.visibility = data.binding.visibility;
        entry.bindingArraySize = data.binding.bindingArraySize;
        return FillEntry(data.data);
    }

    MaybeError operator()(const std::monostate&) {
        return DAWN_INTERNAL_ERROR("invalid bind group layout entry");
    }

  private:
    MaybeError FillEntry(const schema::BindGroupLayoutEntryTypeBufferBindingData& data) {
        entry.buffer = {
            .type = data.type,
            .hasDynamicOffset = data.hasDynamicOffset,
            .minBindingSize = data.minBindingSize,
        };
        return {};
    }

    MaybeError FillEntry(const schema::BindGroupLayoutEntryTypeSamplerBindingData& data) {
        entry.sampler = {.type = data.type};
        return {};
    }

    MaybeError FillEntry(const schema::BindGroupLayoutEntryTypeStorageTextureBindingData& data) {
        entry.storageTexture = {
            .access = data.access,
            .format = data.format,
            .viewDimension = data.viewDimension,
        };
        return {};
    }

    MaybeError FillEntry(const schema::BindGroupLayoutEntryTypeTextureBindingData& data) {
        entry.texture = {
            .sampleType = data.sampleType,
            .viewDimension = data.viewDimension,
            .multisampled = data.multisampled,
        };
        return {};
    }

    MaybeError FillEntry(const schema::BindGroupLayoutEntryTypeExternalTextureBindingData& data) {
        entry.nextInChain = &externalTextureBindingLayout;
        return {};
    }

    template <typename T>
    MaybeError FillEntry(const T&) {
        return DAWN_INTERNAL_ERROR("unhandled bind group layout entry type");
    }
};

ResultOrError<wgpu::BindGroup> CreateResource(const DawnRootCommandVisitor& replay,
                                              wgpu::Device device,
                                              const BindGroupData& data,
                                              const std::string& label) {
    std::vector<wgpu::BindGroupEntry> entries;
    entries.reserve(data.entries.size());
    std::vector<std::unique_ptr<wgpu::ExternalTextureBindingEntry>> externalTextureBindingEntries;

    for (const auto& entryVariant : data.entries) {
        DAWN_TRY(std::visit(BindGroupEntryVisitor{replay, entries, externalTextureBindingEntries},
                            entryVariant));
    }

    wgpu::BindGroupDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::BindGroupLayout>(data.bg.layoutId),
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&desc);
    return {bindGroup};
}

ResultOrError<wgpu::BindGroupLayout> CreateResource(const DawnRootCommandVisitor& replay,
                                                    wgpu::Device device,
                                                    const BindGroupLayoutData& data,
                                                    const std::string& label) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    entries.reserve(data.entries.size());

    // External texture binding layouts are chained structs that are set as a pointer within
    // the bind group layout entry. We declare an entry here so that it can be used when needed
    // in each BindGroupLayoutEntry and so it can stay alive until the call to
    // device.CreateBindGroupLayout. Because ExternalTextureBindingLayout is an empty struct,
    // there's no issue with using the same struct multiple times.
    wgpu::ExternalTextureBindingLayout externalTextureBindingLayout;

    for (const auto& entryVariant : data.entries) {
        wgpu::BindGroupLayoutEntry entry;
        DAWN_TRY(std::visit(BindGroupLayoutEntryVisitor{entry, externalTextureBindingLayout},
                            entryVariant));
        entries.push_back(entry);
    }

    wgpu::BindGroupLayoutDescriptor desc{
        .label = wgpu::StringView(label),
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&desc);
    return {bindGroupLayout};
}

ResultOrError<wgpu::Buffer> CreateResource(const DawnRootCommandVisitor&,
                                           wgpu::Device device,
                                           const schema::Buffer& buf,
                                           const std::string& label) {
    wgpu::BufferUsage usage =
        (buf.usage & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite))
            ? buf.usage
            : (buf.usage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Remap mappable write buffers as CopySrc|CopyDst as we use WriteBuffer to set their contents.
    if (usage == (wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc)) {
        usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    }

    wgpu::BufferDescriptor desc{
        .label = wgpu::StringView(label),
        .usage = usage,
        .size = buf.size,
    };
    wgpu::Buffer buffer = device.CreateBuffer(&desc);
    return {buffer};
}

ResultOrError<wgpu::ComputePipeline> CreateResource(const DawnRootCommandVisitor& replay,
                                                    wgpu::Device device,
                                                    const schema::ComputePipeline& pipeline,
                                                    const std::string& label) {
    std::vector<wgpu::ConstantEntry> constants = ToWGPU(pipeline.compute.constants);

    wgpu::ComputePipelineDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::PipelineLayout>(pipeline.layoutId),
        .compute =
            {
                .module = replay.GetObjectById<wgpu::ShaderModule>(pipeline.compute.moduleId),
                .entryPoint = wgpu::StringView(pipeline.compute.entryPoint),
                .constantCount = constants.size(),
                .constants = constants.data(),
            },
    };
    wgpu::ComputePipeline computePipeline = device.CreateComputePipeline(&desc);
    return {computePipeline};
}

ResultOrError<wgpu::ExternalTexture> CreateResource(const DawnRootCommandVisitor& replay,
                                                    wgpu::Device device,
                                                    const schema::ExternalTexture& tex,
                                                    const std::string& label) {
    wgpu::ExternalTextureDescriptor desc{
        .label = wgpu::StringView(label),
        .plane0 = replay.GetObjectById<wgpu::TextureView>(tex.plane0Id),
        .plane1 = replay.GetObjectById<wgpu::TextureView>(tex.plane1Id),
        .cropOrigin = ToWGPU(tex.cropOrigin),
        .cropSize = ToWGPU(tex.cropSize),
        .apparentSize = ToWGPU(tex.apparentSize),
        .doYuvToRgbConversionOnly = tex.doYuvToRgbConversionOnly,
        .yuvToRgbConversionMatrix = tex.yuvToRgbConversionMatrix.data(),
        .srcTransferFunctionParameters = tex.srcTransferFunctionParameters.data(),
        .dstTransferFunctionParameters = tex.dstTransferFunctionParameters.data(),
        .gamutConversionMatrix = tex.gamutConversionMatrix.data(),
        .mirrored = tex.mirrored,
        .rotation = tex.rotation,
    };
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&desc);
    return {externalTexture};
}

ResultOrError<wgpu::PipelineLayout> CreateResource(const DawnRootCommandVisitor& replay,
                                                   wgpu::Device device,
                                                   const schema::PipelineLayout& layout,
                                                   const std::string& label) {
    std::vector<wgpu::BindGroupLayout> bindGroupLayouts;
    for (const auto bindGroupLayoutId : layout.bindGroupLayoutIds) {
        bindGroupLayouts.push_back(replay.GetObjectById<wgpu::BindGroupLayout>(bindGroupLayoutId));
    }

    wgpu::PipelineLayoutDescriptor desc{
        .label = wgpu::StringView(label),
        .bindGroupLayoutCount = bindGroupLayouts.size(),
        .bindGroupLayouts = bindGroupLayouts.data(),
        .immediateSize = layout.immediateSize,
    };
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&desc);
    return {pipelineLayout};
}

ResultOrError<wgpu::QuerySet> CreateResource(const DawnRootCommandVisitor& replay,
                                             wgpu::Device device,
                                             const schema::QuerySet& querySetData,
                                             const std::string& label) {
    wgpu::QuerySetDescriptor desc{
        .label = wgpu::StringView(label),
        .type = querySetData.type,
        .count = querySetData.count,
    };
    wgpu::QuerySet querySet = device.CreateQuerySet(&desc);
    return {querySet};
}

template <typename Base, typename Pass>
struct DawnCommandVisitor : Base {
    const DawnRootCommandVisitor& replay;
    Pass pass;

    DawnCommandVisitor(const DawnRootCommandVisitor& r, Pass p) : replay(r), pass(p) {}

    MaybeError operator()(const schema::CommandBufferCommandSetBindGroupCmdData& data) override {
        pass.SetBindGroup(data.index, replay.GetObjectById<wgpu::BindGroup>(data.bindGroupId),
                          data.dynamicOffsets.size(), data.dynamicOffsets.data());
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandSetImmediatesCmdData& data) override {
        pass.SetImmediates(data.offset, data.data.data(), data.data.size());
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandPushDebugGroupCmdData& data) override {
        pass.PushDebugGroup(wgpu::StringView(data.groupLabel));
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandInsertDebugMarkerCmdData& data) override {
        pass.InsertDebugMarker(wgpu::StringView(data.markerLabel));
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandPopDebugGroupCmdData& data) override {
        pass.PopDebugGroup();
        return {};
    }
};

template <typename Base, typename Pass>
struct DawnRenderVisitor : DawnCommandVisitor<Base, Pass> {
    using DawnCommandVisitor<Base, Pass>::DawnCommandVisitor;
    using DawnCommandVisitor<Base, Pass>::replay;
    using DawnCommandVisitor<Base, Pass>::pass;

    MaybeError operator()(
        const schema::CommandBufferCommandSetRenderPipelineCmdData& data) override {
        pass.SetPipeline(replay.template GetObjectById<wgpu::RenderPipeline>(data.pipelineId));
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandSetVertexBufferCmdData& data) override {
        pass.SetVertexBuffer(data.slot, replay.template GetObjectById<wgpu::Buffer>(data.bufferId),
                             data.offset, data.size);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandSetIndexBufferCmdData& data) override {
        pass.SetIndexBuffer(replay.template GetObjectById<wgpu::Buffer>(data.bufferId), data.format,
                            data.offset, data.size);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandDrawCmdData& data) override {
        pass.Draw(data.vertexCount, data.instanceCount, data.firstVertex, data.firstInstance);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandDrawIndexedCmdData& data) override {
        pass.DrawIndexed(data.indexCount, data.instanceCount, data.firstIndex, data.baseVertex,
                         data.firstInstance);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandDrawIndirectCmdData& data) override {
        pass.DrawIndirect(replay.template GetObjectById<wgpu::Buffer>(data.indirectBufferId),
                          data.indirectOffset);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandDrawIndexedIndirectCmdData& data) override {
        pass.DrawIndexedIndirect(replay.template GetObjectById<wgpu::Buffer>(data.indirectBufferId),
                                 data.indirectOffset);
        return {};
    }
};

struct DawnRenderBundleVisitor : DawnRenderVisitor<RenderBundleVisitor, wgpu::RenderBundleEncoder> {
    using DawnRenderVisitor::DawnRenderVisitor;

    MaybeError operator()(const schema::CommandBufferCommandEndCmdData& data) override {
        return {};
    }
};

ResultOrError<wgpu::RenderBundle> CreateResource(const DawnRootCommandVisitor& replay,
                                                 wgpu::Device device,
                                                 const RenderBundleData& data,
                                                 const std::string& label) {
    wgpu::RenderBundleEncoderDescriptor desc{
        .colorFormatCount = data.bundle.colorFormats.size(),
        .colorFormats = data.bundle.colorFormats.data(),
        .depthStencilFormat = data.bundle.depthStencilFormat,
        .sampleCount = data.bundle.sampleCount,
        .depthReadOnly = data.bundle.depthReadOnly,
        .stencilReadOnly = data.bundle.stencilReadOnly,
    };
    wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&desc);
    DawnRenderBundleVisitor visitor{replay, encoder};
    DAWN_TRY(ProcessRenderBundleCommands(data.readHead, &visitor));

    wgpu::RenderBundleDescriptor bundleDesc{
        .label = wgpu::StringView(label),
    };
    wgpu::RenderBundle renderBundle = encoder.Finish(&bundleDesc);
    return {renderBundle};
}

ResultOrError<wgpu::RenderPipeline> CreateResource(const DawnRootCommandVisitor& replay,
                                                   wgpu::Device device,
                                                   const schema::RenderPipeline& pipeline,
                                                   const std::string& label) {
    std::vector<wgpu::ConstantEntry> vertexConstants = ToWGPU(pipeline.vertex.program.constants);
    std::vector<wgpu::ConstantEntry> fragmentConstants =
        ToWGPU(pipeline.fragment.program.constants);
    std::vector<wgpu::ColorTargetState> colorTargets;
    std::array<wgpu::ColorTargetStateExpandResolveTextureDawn, kMaxColorAttachments>
        expandResolveChains;
    std::vector<wgpu::BlendState> blendStates(pipeline.fragment.targets.size());
    std::vector<wgpu::VertexBufferLayout> buffers;

    std::vector<wgpu::VertexAttribute> attributes(kMaxVertexAttributes);
    uint32_t attributeCount = 0;

    for (const auto& buffer : pipeline.vertex.buffers) {
        const auto attributesForBuffer = &attributes[attributeCount];
        for (const auto& attrib : buffer.attributes) {
            auto& attr = attributes[attributeCount++];
            attr.format = attrib.format;
            attr.offset = attrib.offset;
            attr.shaderLocation = attrib.shaderLocation;
        }
        buffers.push_back({
            .stepMode = buffer.stepMode,
            .arrayStride = buffer.arrayStride,
            .attributeCount = buffer.attributes.size(),
            .attributes = attributesForBuffer,
        });
    }

    wgpu::FragmentState* fragment = nullptr;
    wgpu::FragmentState fragmentState;
    if (pipeline.fragment.program.moduleId) {
        fragment = &fragmentState;
        fragmentState.module =
            replay.GetObjectById<wgpu::ShaderModule>(pipeline.fragment.program.moduleId);
        fragmentState.entryPoint = wgpu::StringView(pipeline.fragment.program.entryPoint);
        fragmentState.constantCount = fragmentConstants.size();
        fragmentState.constants = fragmentConstants.data();
        for (const auto& target : pipeline.fragment.targets) {
            wgpu::BlendState& blend = blendStates[colorTargets.size()];
            blend = ToWGPU(target.blend);

            wgpu::ChainedStruct* nextInChain = nullptr;
            if (target.expandResolveMode != schema::ExpandResolveMode::Unused) {
                auto expandResolve = &expandResolveChains[colorTargets.size()];
                expandResolve->enabled =
                    target.expandResolveMode == schema::ExpandResolveMode::Enabled;
                nextInChain = expandResolve;
            }
            colorTargets.push_back({
                .nextInChain = nextInChain,
                .format = target.format,
                .blend = IsBlendEnabled(blend) ? &blend : nullptr,
                .writeMask = target.writeMask,
            });
        }
        fragmentState.targetCount = colorTargets.size();
        fragmentState.targets = colorTargets.data();
    }

    wgpu::DepthStencilState* depthStencil = nullptr;
    wgpu::DepthStencilState depthStencilState;
    if (pipeline.depthStencil.format != wgpu::TextureFormat::Undefined) {
        depthStencil = &depthStencilState;
        depthStencilState.format = pipeline.depthStencil.format;
        depthStencilState.depthWriteEnabled = pipeline.depthStencil.depthWriteEnabled;
        depthStencilState.depthCompare = pipeline.depthStencil.depthCompare;
        depthStencilState.stencilFront = ToWGPU(pipeline.depthStencil.stencilFront);
        depthStencilState.stencilBack = ToWGPU(pipeline.depthStencil.stencilBack);
        depthStencilState.stencilReadMask = pipeline.depthStencil.stencilReadMask;
        depthStencilState.stencilWriteMask = pipeline.depthStencil.stencilWriteMask;
        depthStencilState.depthBias = pipeline.depthStencil.depthBias;
        depthStencilState.depthBiasSlopeScale = pipeline.depthStencil.depthBiasSlopeScale;
        depthStencilState.depthBiasClamp = pipeline.depthStencil.depthBiasClamp;
    }

    wgpu::RenderPipelineDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::PipelineLayout>(pipeline.layoutId),
        .vertex =
            {
                .module =
                    replay.GetObjectById<wgpu::ShaderModule>(pipeline.vertex.program.moduleId),
                .entryPoint = wgpu::StringView(pipeline.vertex.program.entryPoint),
                .constantCount = vertexConstants.size(),
                .constants = vertexConstants.data(),
                .bufferCount = buffers.size(),
                .buffers = buffers.data(),
            },
        .primitive =
            {
                .topology = pipeline.primitive.topology,
                .stripIndexFormat = pipeline.primitive.stripIndexFormat,
                .frontFace = pipeline.primitive.frontFace,
                .cullMode = pipeline.primitive.cullMode,
                .unclippedDepth = pipeline.primitive.unclippedDepth,
            },
        .depthStencil = depthStencil,
        .multisample =
            {
                .count = pipeline.multisample.count,
                .mask = pipeline.multisample.mask,
                .alphaToCoverageEnabled = pipeline.multisample.alphaToCoverageEnabled,
            },
        .fragment = fragment,
    };
    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&desc);
    return {renderPipeline};
}

ResultOrError<wgpu::Sampler> CreateResource(const DawnRootCommandVisitor&,
                                            wgpu::Device device,
                                            const schema::Sampler& sampler,
                                            const std::string& label) {
    wgpu::SamplerDescriptor desc{
        .label = wgpu::StringView(label),
        .addressModeU = sampler.addressModeU,
        .addressModeV = sampler.addressModeV,
        .addressModeW = sampler.addressModeW,
        .magFilter = sampler.magFilter,
        .minFilter = sampler.minFilter,
        .mipmapFilter = sampler.mipmapFilter,
        .lodMinClamp = sampler.lodMinClamp,
        .lodMaxClamp = sampler.lodMaxClamp,
        .compare = sampler.compare,
        .maxAnisotropy = sampler.maxAnisotropy,
    };
    return {device.CreateSampler(&desc)};
}

ResultOrError<wgpu::ShaderModule> CreateResource(const DawnRootCommandVisitor&,
                                                 wgpu::Device device,
                                                 const schema::ShaderModule& mod,
                                                 const std::string& label) {
    // TODO(452840621): Make this use a chain instead of hard coded to WGSL only and handle other
    // chained structs.
    wgpu::ShaderSourceWGSL source({
        .nextInChain = nullptr,
        .code = wgpu::StringView(mod.code),
    });
    wgpu::ShaderModuleDescriptor desc{
        .nextInChain = &source,
        .label = wgpu::StringView(label),
    };
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&desc);
    return {shaderModule};
}

ResultOrError<wgpu::TexelBufferView> CreateResource(const DawnRootCommandVisitor&,
                                                    wgpu::Device device,
                                                    const schema::TexelBufferView& view,
                                                    const std::string& label) {
    return DAWN_UNIMPLEMENTED_ERROR("TexelBufferView not supported");
}

ResultOrError<wgpu::Texture> CreateResource(const DawnRootCommandVisitor&,
                                            wgpu::Device device,
                                            const schema::Texture& tex,
                                            const std::string& label) {
    wgpu::TextureUsage usage = tex.usage;
    if (!(usage & wgpu::TextureUsage::TransientAttachment)) {
        usage |= wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    }

    wgpu::TextureDescriptor desc{
        .label = wgpu::StringView(label),
        .usage = usage,
        .dimension = tex.dimension,
        .size = ToWGPU(tex.size),
        .format = tex.format,
        .mipLevelCount = tex.mipLevelCount,
        .sampleCount = tex.sampleCount,
        .viewFormatCount = tex.viewFormats.size(),
        .viewFormats = tex.viewFormats.data(),
    };
    wgpu::Texture texture = device.CreateTexture(&desc);
    return {texture};
}

ResultOrError<wgpu::TextureView> CreateResource(const DawnRootCommandVisitor& replay,
                                                wgpu::Device device,
                                                const schema::TextureView& view,
                                                const std::string& label) {
    wgpu::TextureViewDescriptor desc{
        .label = wgpu::StringView(label),
        .format = view.format,
        .dimension = view.dimension,
        .baseMipLevel = view.baseMipLevel,
        .mipLevelCount = view.mipLevelCount,
        .baseArrayLayer = view.baseArrayLayer,
        .arrayLayerCount = view.arrayLayerCount,
        .aspect = view.aspect,
        .usage = view.usage,
    };

    wgpu::TextureComponentSwizzleDescriptor swizzleDesc = {};
    swizzleDesc.swizzle.r = view.swizzle.r;
    swizzleDesc.swizzle.g = view.swizzle.g;
    swizzleDesc.swizzle.b = view.swizzle.b;
    swizzleDesc.swizzle.a = view.swizzle.a;
    if (!IsSwizzleIdentity(swizzleDesc.swizzle)) {
        desc.nextInChain = &swizzleDesc;
    }

    wgpu::Texture texture = replay.GetObjectById<wgpu::Texture>(view.textureId);
    wgpu::TextureView textureView = texture.CreateView(&desc);
    return {textureView};
}

struct DawnComputePassVisitor : DawnCommandVisitor<ComputePassVisitor, wgpu::ComputePassEncoder> {
    using DawnCommandVisitor::DawnCommandVisitor;

    MaybeError operator()(
        const schema::CommandBufferCommandSetComputePipelineCmdData& data) override {
        pass.SetPipeline(replay.GetObjectById<wgpu::ComputePipeline>(data.pipelineId));
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandDispatchCmdData& data) override {
        pass.DispatchWorkgroups(data.x, data.y, data.z);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandDispatchIndirectCmdData& data) override {
        pass.DispatchWorkgroupsIndirect(replay.GetObjectById<wgpu::Buffer>(data.bufferId),
                                        data.offset);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandWriteTimestampCmdData& data) override {
        pass.WriteTimestamp(replay.GetObjectById<wgpu::QuerySet>(data.querySetId), data.queryIndex);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandEndCmdData& data) override {
        pass.End();
        return {};
    }
};

struct DawnRenderPassVisitor : DawnRenderVisitor<RenderPassVisitor, wgpu::RenderPassEncoder> {
    using DawnRenderVisitor::DawnRenderVisitor;

    MaybeError operator()(const schema::CommandBufferCommandExecuteBundlesCmdData& data) override {
        std::vector<wgpu::RenderBundle> bundles;
        for (auto bundleId : data.bundleIds) {
            bundles.push_back(replay.GetObjectById<wgpu::RenderBundle>(bundleId));
        }
        pass.ExecuteBundles(bundles.size(), bundles.data());
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandBeginOcclusionQueryCmdData& data) override {
        pass.BeginOcclusionQuery(data.queryIndex);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandEndOcclusionQueryCmdData& data) override {
        pass.EndOcclusionQuery();
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandSetBlendConstantCmdData& data) override {
        wgpu::Color color = ToWGPU(data.color);
        pass.SetBlendConstant(&color);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandSetScissorRectCmdData& data) override {
        pass.SetScissorRect(data.x, data.y, data.width, data.height);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandSetStencilReferenceCmdData& data) override {
        pass.SetStencilReference(data.reference);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandSetViewportCmdData& data) override {
        pass.SetViewport(data.x, data.y, data.width, data.height, data.minDepth, data.maxDepth);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandWriteTimestampCmdData& data) override {
        pass.WriteTimestamp(replay.GetObjectById<wgpu::QuerySet>(data.querySetId), data.queryIndex);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandEndCmdData& data) override {
        pass.End();
        return {};
    }
};

struct DawnEncoderVisitor : EncoderVisitor {
    const DawnRootCommandVisitor& replay;
    wgpu::CommandEncoder pass;
    wgpu::Device device;

    DawnEncoderVisitor(const DawnRootCommandVisitor& r, wgpu::CommandEncoder p, wgpu::Device d)
        : replay(r), pass(p), device(d) {}

    ResultOrError<ComputePassVisitor*> BeginComputePass(
        const schema::CommandBufferCommandBeginComputePassCmdData& data) override {
        wgpu::PassTimestampWrites timestampWrites = ToWGPU(replay, data.timestampWrites);
        wgpu::ComputePassDescriptor desc{
            .label = wgpu::StringView(data.label),
            .timestampWrites = timestampWrites.querySet ? &timestampWrites : nullptr,
        };
        mComputePassEncoder = pass.BeginComputePass(&desc);
        mComputePassVisitor = std::make_unique<DawnComputePassVisitor>(replay, mComputePassEncoder);
        return mComputePassVisitor.get();
    }

    ResultOrError<RenderPassVisitor*> BeginRenderPass(
        const schema::CommandBufferCommandBeginRenderPassCmdData& data) override {
        wgpu::PassTimestampWrites timestampWrites = ToWGPU(replay, data.timestampWrites);
        std::vector<wgpu::RenderPassColorAttachment> colorAttachments;

        for (const auto& attachment : data.colorAttachments) {
            colorAttachments.push_back(wgpu::RenderPassColorAttachment{
                .nextInChain = nullptr,
                .view = replay.GetObjectById<wgpu::TextureView>(attachment.viewId),
                .depthSlice = attachment.depthSlice,
                .resolveTarget =
                    replay.GetObjectById<wgpu::TextureView>(attachment.resolveTargetId),
                .loadOp = attachment.loadOp,
                .storeOp = attachment.storeOp,
                .clearValue = ToWGPU(attachment.clearValue),
            });
        }

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment{
            .view = replay.GetObjectById<wgpu::TextureView>(data.depthStencilAttachment.viewId),
            .depthLoadOp = data.depthStencilAttachment.depthLoadOp,
            .depthStoreOp = data.depthStencilAttachment.depthStoreOp,
            .depthClearValue = data.depthStencilAttachment.depthClearValue,
            .depthReadOnly = data.depthStencilAttachment.depthReadOnly,
            .stencilLoadOp = data.depthStencilAttachment.stencilLoadOp,
            .stencilStoreOp = data.depthStencilAttachment.stencilStoreOp,
            .stencilClearValue = data.depthStencilAttachment.stencilClearValue,
            .stencilReadOnly = data.depthStencilAttachment.stencilReadOnly,
        };

        wgpu::RenderPassDescriptor desc{
            .label = wgpu::StringView(data.label),
            .colorAttachmentCount = colorAttachments.size(),
            .colorAttachments = colorAttachments.data(),
            .depthStencilAttachment =
                depthStencilAttachment.view != nullptr ? &depthStencilAttachment : nullptr,
            .occlusionQuerySet = replay.GetObjectById<wgpu::QuerySet>(data.occlusionQuerySetId),
            .timestampWrites = timestampWrites.querySet ? &timestampWrites : nullptr,
        };

        wgpu::RenderPassDescriptorResolveRect resolveRect;
        if (data.resolveRect.width != 0) {
            resolveRect.colorOffsetX = data.resolveRect.colorOffsetX;
            resolveRect.colorOffsetY = data.resolveRect.colorOffsetY;
            resolveRect.resolveOffsetX = data.resolveRect.resolveOffsetX;
            resolveRect.resolveOffsetY = data.resolveRect.resolveOffsetY;
            resolveRect.width = data.resolveRect.width;
            resolveRect.height = data.resolveRect.height;
            desc.nextInChain = &resolveRect;
        }

        mRenderPassEncoder = pass.BeginRenderPass(&desc);
        mRenderPassVisitor = std::make_unique<DawnRenderPassVisitor>(replay, mRenderPassEncoder);
        return mRenderPassVisitor.get();
    }

    MaybeError operator()(const schema::CommandBufferCommandClearBufferCmdData& data) override {
        pass.ClearBuffer(replay.GetObjectById<wgpu::Buffer>(data.bufferId), data.offset, data.size);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandCopyBufferToBufferCmdData& data) override {
        pass.CopyBufferToBuffer(
            replay.GetObjectById<wgpu::Buffer>(data.srcBufferId), data.srcOffset,
            replay.GetObjectById<wgpu::Buffer>(data.dstBufferId), data.dstOffset, data.size);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandCopyBufferToTextureCmdData& data) override {
        wgpu::TexelCopyBufferInfo src = ToWGPU(replay, data.source);
        wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, data.destination);
        wgpu::Extent3D copySize = ToWGPU(data.copySize);
        pass.CopyBufferToTexture(&src, &dst, &copySize);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandCopyTextureToBufferCmdData& data) override {
        wgpu::TexelCopyTextureInfo src = ToWGPU(replay, data.source);
        wgpu::TexelCopyBufferInfo dst = ToWGPU(replay, data.destination);
        wgpu::Extent3D copySize = ToWGPU(data.copySize);
        pass.CopyTextureToBuffer(&src, &dst, &copySize);
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandCopyTextureToTextureCmdData& data) override {
        wgpu::TexelCopyTextureInfo src = ToWGPU(replay, data.source);
        wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, data.destination);
        wgpu::Extent3D copySize = ToWGPU(data.copySize);
        pass.CopyTextureToTexture(&src, &dst, &copySize);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandResolveQuerySetCmdData& data) override {
        pass.ResolveQuerySet(
            replay.GetObjectById<wgpu::QuerySet>(data.querySetId), data.firstQuery, data.queryCount,
            replay.GetObjectById<wgpu::Buffer>(data.destinationId), data.destinationOffset);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandWriteBufferCmdData& data) override {
        wgpu::Buffer buffer = replay.GetObjectById<wgpu::Buffer>(data.bufferId);
        pass.WriteBuffer(buffer, data.bufferOffset, data.data.data(), data.data.size());
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandWriteTimestampCmdData& data) override {
        pass.WriteTimestamp(replay.GetObjectById<wgpu::QuerySet>(data.querySetId), data.queryIndex);
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandPushDebugGroupCmdData& data) override {
        pass.PushDebugGroup(wgpu::StringView(data.groupLabel));
        return {};
    }

    MaybeError operator()(
        const schema::CommandBufferCommandInsertDebugMarkerCmdData& data) override {
        pass.InsertDebugMarker(wgpu::StringView(data.markerLabel));
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandPopDebugGroupCmdData& data) override {
        pass.PopDebugGroup();
        return {};
    }

    MaybeError operator()(const schema::CommandBufferCommandEndCmdData& data) override {
        return {};
    }

  private:
    wgpu::ComputePassEncoder mComputePassEncoder;
    std::unique_ptr<DawnComputePassVisitor> mComputePassVisitor;
    wgpu::RenderPassEncoder mRenderPassEncoder;
    std::unique_ptr<DawnRenderPassVisitor> mRenderPassVisitor;
};

ResultOrError<wgpu::CommandBuffer> CreateResource(const DawnRootCommandVisitor& replay,
                                                  wgpu::Device device,
                                                  const CommandBufferData& data,
                                                  const std::string& label) {
    wgpu::CommandEncoderDescriptor desc{
        .label = wgpu::StringView(label),
    };
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&desc);
    DawnEncoderVisitor visitor{replay, encoder, device};
    DAWN_TRY(ProcessEncoderCommands(data.readHead, &visitor));
    return {encoder.Finish()};
}

}  // anonymous namespace

MaybeError DawnResourceVisitor::operator()(const BindGroupData& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const BindGroupLayoutData& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::Buffer& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const CommandBufferData& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::ComputePipeline& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::ExternalTexture& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::PipelineLayout& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::QuerySet& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const RenderBundleData& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::RenderPipeline& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::Sampler& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::ShaderModule& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::TexelBufferView& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::Texture& data) {
    return Create(data);
}
MaybeError DawnResourceVisitor::operator()(const schema::TextureView& data) {
    return Create(data);
}

template <typename T>
MaybeError DawnResourceVisitor::Create(const T& data) {
    auto res =
        CreateResource(*mVisitor, mVisitor->GetDevice(), data, mVisitor->GetCurrentResourceLabel());
    if (res.IsError()) {
        return res.AcquireError();
    }
    mVisitor->AddResource(mVisitor->GetCurrentResourceId(), mVisitor->GetCurrentResourceLabel(),
                          res.AcquireSuccess());
    return {};
}

MaybeError DawnRootCommandVisitor::operator()(const schema::RootCommandWriteBufferCmdData& data) {
    wgpu::Buffer buffer = GetObjectById<wgpu::Buffer>(data.bufferId);
    return ReadContentIntoBuffer(*mContentReadHead, mDevice, buffer, data.bufferOffset, data.size);
}

MaybeError DawnRootCommandVisitor::operator()(const schema::RootCommandWriteTextureCmdData& data) {
    return ReadContentIntoTexture(*this, *mContentReadHead, mDevice, data);
}

MaybeError DawnRootCommandVisitor::operator()(const schema::RootCommandQueueSubmitCmdData& data) {
    std::vector<wgpu::CommandBuffer> commandBuffers;
    commandBuffers.reserve(data.commandBuffers.size());
    for (auto id : data.commandBuffers) {
        commandBuffers.push_back(GetObjectById<wgpu::CommandBuffer>(id));
    }
    mDevice.GetQueue().Submit(commandBuffers.size(), commandBuffers.data());
    return {};
}

MaybeError DawnRootCommandVisitor::operator()(const schema::RootCommandSetLabelCmdData& data) {
    return SetLabel(data.id, data.type, data.label);
}

MaybeError DawnRootCommandVisitor::operator()(const schema::RootCommandInitTextureCmdData& data) {
    return InitializeTexture(*this, mBlitBufferToDepthTexture, *mContentReadHead, mDevice, data);
}

MaybeError DawnRootCommandVisitor::SetLabel(schema::ObjectId id,
                                            schema::ObjectType type,
                                            const std::string& label) {
// We update both the object's label and our own copy of the label
// as there is no API to get an object's label from WebGPU
#define DAWN_REPLAY_GET_X_MACRO(_1, _2, NAME, ...) NAME

#define DAWN_SET_LABEL_CASE_INVALID(NAME, ...) \
    case schema::ObjectType::NAME:             \
        break;

#define DAWN_SET_LABEL_CASE_VALID(NAME, ...)                                                \
    case schema::ObjectType::NAME: {                                                        \
        auto iter = mResources.find(id);                                                    \
        DAWN_ASSERT(iter != mResources.end());                                              \
        std::get_if<wgpu::NAME>(&iter->second.resource)->SetLabel(wgpu::StringView(label)); \
        iter->second.label = label;                                                         \
        break;                                                                              \
    }

#define DAWN_SET_LABEL_CASE(...)                                                                 \
    DAWN_REPLAY_GET_X_MACRO(__VA_ARGS__, DAWN_SET_LABEL_CASE_INVALID, DAWN_SET_LABEL_CASE_VALID) \
    (__VA_ARGS__)

#define DAWN_SET_LABEL_GEN(NAME, MEMBERS) MEMBERS(DAWN_SET_LABEL_CASE)

    switch (type) {
        DAWN_REPLAY_OBJECT_TYPES_ENUM(DAWN_SET_LABEL_GEN)
        default:
            return DAWN_INTERNAL_ERROR("unhandled resource type");
    }

#undef DAWN_SET_LABEL_GEN
#undef DAWN_SET_LABEL_CASE
#undef DAWN_SET_LABEL_CASE_VALID
#undef DAWN_SET_LABEL_CASE_INVALID
#undef DAWN_REPLAY_GET_X_MACRO

    return {};
}

std::unique_ptr<ReplayImpl> ReplayImpl::Create(wgpu::Device device,
                                               std::unique_ptr<Capture> capture) {
    auto captureImpl = std::unique_ptr<CaptureImpl>(static_cast<CaptureImpl*>(capture.release()));
    return std::unique_ptr<ReplayImpl>(new ReplayImpl(device, std::move(captureImpl)));
}

ReplayImpl::ReplayImpl(wgpu::Device device, std::unique_ptr<CaptureImpl> capture)
    : mVisitor(new DawnRootCommandVisitor(device)), mCapture(std::move(capture)) {
    mVisitor->AddResource(schema::kDeviceId, "", device);
}

MaybeError ReplayImpl::Play() {
    if (mCapture->Walk(*mVisitor)) {
        return {};
    }
    return DAWN_INTERNAL_ERROR("Capture walk failed");
}

template <typename T>
T ReplayImpl::GetObjectByLabel(std::string_view label) const {
    return mVisitor->GetObjectByLabel<T>(label);
}

template <typename T>
T ReplayImpl::GetObjectById(schema::ObjectId id) const {
    return mVisitor->GetObjectById<T>(id);
}

template <typename T>
const std::string& ReplayImpl::GetLabel(const T& object) const {
    return mVisitor->GetLabel(object);
}

const std::string& ReplayImpl::GetLabel(schema::ObjectId id) const {
    return mVisitor->GetLabel(id);
}

template wgpu::BindGroup ReplayImpl::GetObjectByLabel<wgpu::BindGroup>(
    std::string_view label) const;
template wgpu::BindGroupLayout ReplayImpl::GetObjectByLabel<wgpu::BindGroupLayout>(
    std::string_view label) const;
template wgpu::Buffer ReplayImpl::GetObjectByLabel<wgpu::Buffer>(std::string_view label) const;
template wgpu::CommandBuffer ReplayImpl::GetObjectByLabel<wgpu::CommandBuffer>(
    std::string_view label) const;
template wgpu::ComputePipeline ReplayImpl::GetObjectByLabel<wgpu::ComputePipeline>(
    std::string_view label) const;
template wgpu::Device ReplayImpl::GetObjectByLabel<wgpu::Device>(std::string_view label) const;
template wgpu::PipelineLayout ReplayImpl::GetObjectByLabel<wgpu::PipelineLayout>(
    std::string_view label) const;
template wgpu::QuerySet ReplayImpl::GetObjectByLabel<wgpu::QuerySet>(std::string_view label) const;
template wgpu::RenderBundle ReplayImpl::GetObjectByLabel<wgpu::RenderBundle>(
    std::string_view label) const;
template wgpu::RenderPipeline ReplayImpl::GetObjectByLabel<wgpu::RenderPipeline>(
    std::string_view label) const;
template wgpu::Sampler ReplayImpl::GetObjectByLabel<wgpu::Sampler>(std::string_view label) const;
template wgpu::ShaderModule ReplayImpl::GetObjectByLabel<wgpu::ShaderModule>(
    std::string_view label) const;
template wgpu::Texture ReplayImpl::GetObjectByLabel<wgpu::Texture>(std::string_view label) const;
template wgpu::ExternalTexture ReplayImpl::GetObjectByLabel<wgpu::ExternalTexture>(
    std::string_view label) const;
template wgpu::TextureView ReplayImpl::GetObjectByLabel<wgpu::TextureView>(
    std::string_view label) const;

template wgpu::BindGroup ReplayImpl::GetObjectById<wgpu::BindGroup>(schema::ObjectId id) const;
template wgpu::BindGroupLayout ReplayImpl::GetObjectById<wgpu::BindGroupLayout>(
    schema::ObjectId id) const;
template wgpu::Buffer ReplayImpl::GetObjectById<wgpu::Buffer>(schema::ObjectId id) const;
template wgpu::CommandBuffer ReplayImpl::GetObjectById<wgpu::CommandBuffer>(
    schema::ObjectId id) const;
template wgpu::ComputePipeline ReplayImpl::GetObjectById<wgpu::ComputePipeline>(
    schema::ObjectId id) const;
template wgpu::Device ReplayImpl::GetObjectById<wgpu::Device>(schema::ObjectId id) const;
template wgpu::PipelineLayout ReplayImpl::GetObjectById<wgpu::PipelineLayout>(
    schema::ObjectId id) const;
template wgpu::QuerySet ReplayImpl::GetObjectById<wgpu::QuerySet>(schema::ObjectId id) const;
template wgpu::RenderBundle ReplayImpl::GetObjectById<wgpu::RenderBundle>(
    schema::ObjectId id) const;
template wgpu::RenderPipeline ReplayImpl::GetObjectById<wgpu::RenderPipeline>(
    schema::ObjectId id) const;
template wgpu::Sampler ReplayImpl::GetObjectById<wgpu::Sampler>(schema::ObjectId id) const;
template wgpu::ShaderModule ReplayImpl::GetObjectById<wgpu::ShaderModule>(
    schema::ObjectId id) const;
template wgpu::Texture ReplayImpl::GetObjectById<wgpu::Texture>(schema::ObjectId id) const;
template wgpu::ExternalTexture ReplayImpl::GetObjectById<wgpu::ExternalTexture>(
    schema::ObjectId id) const;
template wgpu::TextureView ReplayImpl::GetObjectById<wgpu::TextureView>(schema::ObjectId id) const;

template const std::string& ReplayImpl::GetLabel<wgpu::BindGroup>(const wgpu::BindGroup&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::BindGroupLayout>(
    const wgpu::BindGroupLayout&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::Buffer>(const wgpu::Buffer&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::CommandBuffer>(
    const wgpu::CommandBuffer&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::ComputePipeline>(
    const wgpu::ComputePipeline&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::Device>(const wgpu::Device&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::PipelineLayout>(
    const wgpu::PipelineLayout&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::QuerySet>(const wgpu::QuerySet&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::RenderBundle>(
    const wgpu::RenderBundle&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::RenderPipeline>(
    const wgpu::RenderPipeline&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::Sampler>(const wgpu::Sampler&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::ShaderModule>(
    const wgpu::ShaderModule&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::Texture>(const wgpu::Texture&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::ExternalTexture>(
    const wgpu::ExternalTexture&) const;
template const std::string& ReplayImpl::GetLabel<wgpu::TextureView>(const wgpu::TextureView&) const;

}  // namespace dawn::replay
