// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_TOGGLES_H_
#define SRC_DAWN_NATIVE_TOGGLES_H_

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/native/DawnNative.h"

namespace dawn::native {

struct DawnTogglesDeviceDescriptor;

enum class Toggle {
    EmulateStoreAndMSAAResolve,
    NonzeroClearResourcesOnCreationForTesting,
    AlwaysResolveIntoZeroLevelAndLayer,
    LazyClearResourceOnFirstUse,
    TurnOffVsync,
    UseTemporaryBufferInCompressedTextureToTextureCopy,
    UseD3D12ResourceHeapTier2,
    UseD3D12RenderPass,
    UseD3D12ResidencyManagement,
    DisableResourceSuballocation,
    SkipValidation,
    VulkanUseD32S8,
    VulkanUseS8,
    MetalDisableSamplerCompare,
    MetalUseSharedModeForCounterSampleBuffer,
    DisableBaseVertex,
    DisableBaseInstance,
    DisableIndexedDrawBuffers,
    DisableSnormRead,
    DisableDepthRead,
    DisableStencilRead,
    DisableDepthStencilRead,
    DisableBGRARead,
    DisableSampleVariables,
    UseD3D12SmallShaderVisibleHeapForTesting,
    UseDXC,
    DisableRobustness,
    MetalEnableVertexPulling,
    DisallowUnsafeAPIs,
    FlushBeforeClientWaitSync,
    UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
    EmitHLSLDebugSymbols,
    DisallowSpirv,
    DumpShaders,
    ForceWGSLStep,
    DisableWorkgroupInit,
    DisableSymbolRenaming,
    UseUserDefinedLabelsInBackend,
    UsePlaceholderFragmentInVertexOnlyPipeline,
    FxcOptimizations,
    RecordDetailedTimingInTraceEvents,
    DisableTimestampQueryConversion,
    VulkanUseZeroInitializeWorkgroupMemoryExtension,
    D3D12SplitBufferTextureCopyForRowsPerImagePaddings,
    MetalRenderR8RG8UnormSmallMipToTempTexture,
    DisableBlobCache,
    D3D12ForceClearCopyableDepthStencilTextureOnCreation,
    D3D12DontSetClearValueOnDepthTextureCreation,
    D3D12AlwaysUseTypelessFormatsForCastableTexture,
    D3D12AllocateExtraMemoryFor2DArrayTexture,
    D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset,
    ApplyClearBigIntegerColorValueWithDraw,
    MetalUseMockBlitEncoderForWriteTimestamp,
    VulkanSplitCommandBufferOnDepthStencilComputeSampleAfterRenderPass,
    D3D12Allocate2DTexturewithCopyDstAsCommittedResource,

    EnumCount,
    InvalidEnum = EnumCount,
};

// A wrapper of the bitset to store if a toggle is present or not. This wrapper provides the
// convenience to convert the enums of enum class Toggle to the indices of a bitset.
struct TogglesSet {
    std::bitset<static_cast<size_t>(Toggle::EnumCount)> toggleBitset;

    void Set(Toggle toggle, bool enabled);
    bool Has(Toggle toggle) const;
    std::vector<const char*> GetContainedToggleNames() const;
};

// TripleStateTogglesSet track each toggle with three posible states, i.e. "Not provided" (default),
// "Provided as enabled", and "Provided as disabled". This struct can be used to record the
// user-provided toggles, where some toggles are explicitly enabled or disabled while the other
// toggles are left as default.
struct TripleStateTogglesSet {
    TogglesSet togglesIsProvided;
    TogglesSet providedTogglesEnabled;

    static TripleStateTogglesSet CreateFromTogglesDeviceDescriptor(
        const DawnTogglesDeviceDescriptor* togglesDesc);
    // Provide a single toggle with given state.
    void Set(Toggle toggle, bool enabled);
    bool IsProvided(Toggle toggle) const;
    // Return true if the toggle is provided in enable list, and false otherwise.
    bool IsEnabled(Toggle toggle) const;
    // Return true if the toggle is provided in disable list, and false otherwise.
    bool IsDisabled(Toggle toggle) const;
    std::vector<const char*> GetEnabledToggleNames() const;
    std::vector<const char*> GetDisabledToggleNames() const;
};

const char* ToggleEnumToName(Toggle toggle);

class TogglesInfo {
  public:
    TogglesInfo();
    ~TogglesInfo();

    // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
    // of a toggle supported in Dawn.
    const ToggleInfo* GetToggleInfo(const char* toggleName);
    Toggle ToggleNameToEnum(const char* toggleName);

  private:
    void EnsureToggleNameToEnumMapInitialized();

    bool mToggleNameToEnumMapInitialized = false;
    std::unordered_map<std::string, Toggle> mToggleNameToEnumMap;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TOGGLES_H_
