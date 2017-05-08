// Copyright 2017 The NXT Authors
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

#ifndef BACKEND_COMMON_BUFFER_H_
#define BACKEND_COMMON_BUFFER_H_

#include "Forward.h"
#include "Builder.h"
#include "RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class BufferBase : public RefCounted {
        public:
            BufferBase(BufferBuilder* builder);

            uint32_t GetSize() const;
            nxt::BufferUsageBit GetAllowedUsage() const;
            nxt::BufferUsageBit GetUsage() const;
            static bool IsUsagePossible(nxt::BufferUsageBit allowedUsage, nxt::BufferUsageBit usage);
            bool IsTransitionPossible(nxt::BufferUsageBit usage) const;
            bool IsFrozen() const;
            bool HasFrozenUsage(nxt::BufferUsageBit usage) const;
            void TransitionUsageImpl(nxt::BufferUsageBit usage);

            // NXT API
            BufferViewBuilder* CreateBufferViewBuilder();
            void SetSubData(uint32_t start, uint32_t count, const uint32_t* data);
            void TransitionUsage(nxt::BufferUsageBit usage);
            void FreezeUsage(nxt::BufferUsageBit usage);

        private:
            virtual void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) = 0;

            DeviceBase* device;
            uint32_t size;
            nxt::BufferUsageBit allowedUsage = nxt::BufferUsageBit::None;
            nxt::BufferUsageBit currentUsage = nxt::BufferUsageBit::None;
            bool frozen = false;
    };

    class BufferBuilder : public Builder<BufferBase> {
        public:
            BufferBuilder(DeviceBase* device);

            // NXT API
            void SetAllowedUsage(nxt::BufferUsageBit usage);
            void SetInitialUsage(nxt::BufferUsageBit usage);
            void SetSize(uint32_t size);

        private:
            friend class BufferBase;

            BufferBase* GetResultImpl() override;

            uint32_t size;
            nxt::BufferUsageBit allowedUsage = nxt::BufferUsageBit::None;
            nxt::BufferUsageBit currentUsage = nxt::BufferUsageBit::None;
            int propertiesSet = 0;
    };

    class BufferViewBase : public RefCounted {
        public:
            BufferViewBase(BufferViewBuilder* builder);

            BufferBase* GetBuffer();
            uint32_t GetSize() const;
            uint32_t GetOffset() const;

        private:
            Ref<BufferBase> buffer;
            uint32_t size;
            uint32_t offset;
    };

    class BufferViewBuilder : public Builder<BufferViewBase> {
        public:
            BufferViewBuilder(DeviceBase* device, BufferBase* buffer);

            // NXT API
            void SetExtent(uint32_t offset, uint32_t size);

        private:
            friend class BufferViewBase;

            BufferViewBase* GetResultImpl() override;

            Ref<BufferBase> buffer;
            uint32_t offset = 0;
            uint32_t size = 0;
            int propertiesSet = 0;
    };

}

#endif // BACKEND_COMMON_BUFFER_H_
