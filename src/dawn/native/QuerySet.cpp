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

#include "dawn/native/QuerySet.h"

#include <set>

#include "dawn/native/Device.h"
#include "dawn/native/Features.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {

class ErrorQuerySet final : public QuerySetBase {
  public:
    explicit ErrorQuerySet(DeviceBase* device, const QuerySetDescriptor* descriptor)
        : QuerySetBase(device, descriptor, ObjectBase::kError) {}

  private:
    void DestroyImpl() override { DAWN_UNREACHABLE(); }
};

}  // anonymous namespace

MaybeError ValidateQuerySetDescriptor(DeviceBase* device, const QuerySetDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr");

    DAWN_TRY(ValidateQueryType(descriptor->type));

    DAWN_INVALID_IF(descriptor->count > kMaxQueryCount,
                    "Query count (%u) exceeds the maximum query count (%u).", descriptor->count,
                    kMaxQueryCount);

    switch (descriptor->type) {
        case wgpu::QueryType::Occlusion:
            DAWN_INVALID_IF(descriptor->pipelineStatisticCount != 0,
                            "Pipeline statistics specified for a query of type %s.",
                            descriptor->type);
            break;

        case wgpu::QueryType::PipelineStatistics: {
            // TODO(crbug.com/1177506): Pipeline statistics query is not fully implemented.
            // Allow it only as unsafe until the implementaion is completed.
            DAWN_INVALID_IF(!device->IsToggleEnabled(Toggle::AllowUnsafeAPIs),
                            "Pipeline statistics queries are disallowed because they are not "
                            "fully implemented");

            DAWN_INVALID_IF(
                !device->HasFeature(Feature::PipelineStatisticsQuery),
                "Pipeline statistics query set created without the feature being enabled.");

            DAWN_INVALID_IF(descriptor->pipelineStatisticCount == 0,
                            "Pipeline statistics query set created with 0 statistics.");

            std::set<wgpu::PipelineStatisticName> pipelineStatisticsSet;
            for (uint32_t i = 0; i < descriptor->pipelineStatisticCount; i++) {
                DAWN_TRY(ValidatePipelineStatisticName(descriptor->pipelineStatistics[i]));

                auto [_, inserted] =
                    pipelineStatisticsSet.insert((descriptor->pipelineStatistics[i]));
                DAWN_INVALID_IF(!inserted, "Statistic %s is specified more than once.",
                                descriptor->pipelineStatistics[i]);
            }
        } break;

        case wgpu::QueryType::Timestamp:
            DAWN_INVALID_IF(!device->IsToggleEnabled(Toggle::AllowUnsafeAPIs),
                            "Timestamp queries are disallowed because they may expose precise "
                            "timing information.");

            DAWN_INVALID_IF(!device->HasFeature(Feature::TimestampQuery) &&
                                !device->HasFeature(Feature::TimestampQueryInsidePasses),
                            "Timestamp query set created without the feature being enabled.");

            DAWN_INVALID_IF(descriptor->pipelineStatisticCount != 0,
                            "Pipeline statistics specified for a query of type %s.",
                            descriptor->type);
            break;

        default:
            break;
    }

    return {};
}

QuerySetBase::QuerySetBase(DeviceBase* device, const QuerySetDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label),
      mQueryType(descriptor->type),
      mQueryCount(descriptor->count),
      mState(QuerySetState::Available) {
    for (uint32_t i = 0; i < descriptor->pipelineStatisticCount; i++) {
        mPipelineStatistics.push_back(descriptor->pipelineStatistics[i]);
    }

    mQueryAvailability.resize(descriptor->count);
    GetObjectTrackingList()->Track(this);
}

QuerySetBase::QuerySetBase(DeviceBase* device,
                           const QuerySetDescriptor* descriptor,
                           ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label),
      mQueryType(descriptor->type),
      mQueryCount(descriptor->count) {}

QuerySetBase::~QuerySetBase() {
    // Uninitialized or already destroyed
    DAWN_ASSERT(mState == QuerySetState::Unavailable || mState == QuerySetState::Destroyed);
}

void QuerySetBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped and it
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    mState = QuerySetState::Destroyed;
}

// static
QuerySetBase* QuerySetBase::MakeError(DeviceBase* device, const QuerySetDescriptor* descriptor) {
    return new ErrorQuerySet(device, descriptor);
}

ObjectType QuerySetBase::GetType() const {
    return ObjectType::QuerySet;
}

wgpu::QueryType QuerySetBase::GetQueryType() const {
    return mQueryType;
}

uint32_t QuerySetBase::GetQueryCount() const {
    return mQueryCount;
}

const std::vector<wgpu::PipelineStatisticName>& QuerySetBase::GetPipelineStatistics() const {
    return mPipelineStatistics;
}

const std::vector<bool>& QuerySetBase::GetQueryAvailability() const {
    return mQueryAvailability;
}

void QuerySetBase::SetQueryAvailability(uint32_t index, bool available) {
    mQueryAvailability[index] = available;
}

MaybeError QuerySetBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_INVALID_IF(mState == QuerySetState::Destroyed, "%s used while destroyed.", this);
    return {};
}

void QuerySetBase::APIDestroy() {
    Destroy();
}

wgpu::QueryType QuerySetBase::APIGetType() const {
    return mQueryType;
}

uint32_t QuerySetBase::APIGetCount() const {
    return mQueryCount;
}

}  // namespace dawn::native
