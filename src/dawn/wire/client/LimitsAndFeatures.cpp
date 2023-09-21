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

#include "dawn/wire/client/LimitsAndFeatures.h"

#include "dawn/common/Assert.h"
#include "dawn/wire/SupportedFeatures.h"

namespace dawn::wire::client {

LimitsAndFeatures::LimitsAndFeatures() = default;

LimitsAndFeatures::~LimitsAndFeatures() = default;

bool LimitsAndFeatures::GetLimits(WGPUSupportedLimits* limits) const {
    DAWN_ASSERT(limits != nullptr);
    auto* originalNextInChain = limits->nextInChain;
    *limits = mLimits;
    limits->nextInChain = originalNextInChain;
    // Handle other requiring limits that chained after WGPUSupportedLimits
    for (auto* chain = limits->nextInChain; chain; chain = chain->next) {
        // Store the WGPUChainedStructOut to restore the chain after assignment.
        WGPUChainedStructOut originalChainedStructOut = *chain;
        switch (chain->sType) {
            case (WGPUSType_DawnExperimentalSubgroupLimits): {
                auto* experimentalSubgroupLimits =
                    reinterpret_cast<WGPUDawnExperimentalSubgroupLimits*>(chain);
                // This assignment break the next field of WGPUChainedStructOut head.
                *experimentalSubgroupLimits = mExperimentalSubgroupLimits;
                break;
            }
            default:
                // Fail if unknown sType found.
                return false;
        }
        // Restore the chain.
        *chain = originalChainedStructOut;
    }
    return true;
}

bool LimitsAndFeatures::HasFeature(WGPUFeatureName feature) const {
    return mFeatures.count(feature) != 0;
}

size_t LimitsAndFeatures::EnumerateFeatures(WGPUFeatureName* features) const {
    if (features != nullptr) {
        for (WGPUFeatureName f : mFeatures) {
            *features = f;
            ++features;
        }
    }
    return mFeatures.size();
}

void LimitsAndFeatures::SetLimits(const WGPUSupportedLimits* limits) {
    DAWN_ASSERT(limits != nullptr);
    mLimits = *limits;
    mLimits.nextInChain = nullptr;
    // Handle other limits that chained after WGPUSupportedLimits
    for (auto* chain = limits->nextInChain; chain; chain = chain->next) {
        switch (chain->sType) {
            case (WGPUSType_DawnExperimentalSubgroupLimits): {
                auto* experimentalSubgroupLimits =
                    reinterpret_cast<WGPUDawnExperimentalSubgroupLimits*>(chain);
                mExperimentalSubgroupLimits = *experimentalSubgroupLimits;
                mExperimentalSubgroupLimits.chain.next = nullptr;
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }
}

void LimitsAndFeatures::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    DAWN_ASSERT(features != nullptr || featuresCount == 0);
    for (uint32_t i = 0; i < featuresCount; ++i) {
        // Filter out features that the server supports, but the client does not.
        // (Could be different versions)
        if (!IsFeatureSupported(features[i])) {
            continue;
        }
        mFeatures.insert(features[i]);
    }
}

}  // namespace dawn::wire::client
