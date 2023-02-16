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

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/DawnNative.h"

namespace dawn::native {

struct DawnTogglesDescriptor;

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
    D3D12AllocateExtraMemoryFor2DArrayColorTexture,
    D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset,
    D3D12UseTempBufferInTextureToTextureCopyBetweenDifferentDimensions,
    ApplyClearBigIntegerColorValueWithDraw,
    MetalUseMockBlitEncoderForWriteTimestamp,
    VulkanSplitCommandBufferOnDepthStencilComputeSampleAfterRenderPass,
    D3D12Allocate2DTextureWithCopyDstOrRenderAttachmentAsCommittedResource,
    MetalUseCombinedDepthStencilFormatForStencil8,
    MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats,
    MetalKeepMultisubresourceDepthStencilTexturesInitialized,
    UseBlitForBufferToDepthTextureCopy,
    UseBlitForBufferToStencilTextureCopy,
    UseBlitForDepthTextureToTextureCopyToNonzeroSubresource,
    DisallowDeprecatedAPIs,

    // Unresolved issues.
    NoWorkaroundSampleMaskBecomesZeroForAllButLastColorTarget,
    NoWorkaroundIndirectBaseVertexNotApplied,
    NoWorkaroundDstAlphaBlendDoesNotWork,

    EnumCount,
    InvalidEnum = EnumCount,
};

// A wrapper of the bitset to store if a toggle is present or not. This wrapper provides the
// convenience to convert the enums of enum class Toggle to the indices of a bitset.
struct TogglesSet {
    std::bitset<static_cast<size_t>(Toggle::EnumCount)> bitset;
    using Iterator = BitSetIterator<static_cast<size_t>(Toggle::EnumCount), uint32_t>;

    void Set(Toggle toggle, bool enabled);
    bool Has(Toggle toggle) const;
    size_t Count() const;
    Iterator Iterate() const;
};

namespace stream {
class Sink;
}

// TogglesState hold the actual state of toggles for instances, adapters and devices. Each toggle
// is of one of these states: set/default to enabled/disabled, force set to enabled/disabled, or
// left unset without default value (and thus implicitly disabled).
class TogglesState {
  public:
    // Create an empty toggles state of given stage
    explicit TogglesState(ToggleStage stage);

    // Create a RequiredTogglesSet from a DawnTogglesDescriptor, only considering toggles of
    // required toggle stage.
    static TogglesState CreateFromTogglesDescriptor(const DawnTogglesDescriptor* togglesDesc,
                                                    ToggleStage requiredStage);

    // Set a toggle of the same stage of toggles state stage if and only if it is not already set.
    void Default(Toggle toggle, bool enabled);
    // Force set a toggle of same stage of toggles state stage. A force-set toggle will get
    // inherited to all later stage as forced.
    void ForceSet(Toggle toggle, bool enabled);

    // Return whether the toggle is set or not. Force-set is always treated as set.
    bool IsSet(Toggle toggle) const;
    // Return true if and only if the toggle is set to true.
    bool IsEnabled(Toggle toggle) const;
    ToggleStage GetStage() const;
    std::vector<const char*> GetEnabledToggleNames() const;
    std::vector<const char*> GetDisabledToggleNames() const;

    // Friend definition of StreamIn which can be found by ADL to override stream::StreamIn<T>. This
    // allows writing TogglesState to stream for cache key.
    friend void StreamIn(stream::Sink* sink, const TogglesState& togglesState);

  private:
    // Indicating which stage of toggles state is this object holding for, instance, adapter, or
    // device.
    const ToggleStage mStage;
    TogglesSet mTogglesSet;
    TogglesSet mEnabledToggles;
    TogglesSet mForcedToggles;
};

const char* ToggleEnumToName(Toggle toggle);

class TogglesInfo {
  public:
    TogglesInfo();
    ~TogglesInfo();

    // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
    // of a toggle supported in Dawn.
    const ToggleInfo* GetToggleInfo(const char* toggleName);
    // Used to query the details of a toggle enum. The enum value must not be Toggle::InvalidEnum,
    // as Toggle::InvalidEnum doesn't has corresponding ToggleInfo.
    static const ToggleInfo* GetToggleInfo(Toggle toggle);
    Toggle ToggleNameToEnum(const char* toggleName);

  private:
    void EnsureToggleNameToEnumMapInitialized();

    bool mToggleNameToEnumMapInitialized = false;
    std::unordered_map<std::string, Toggle> mToggleNameToEnumMap;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TOGGLES_H_
