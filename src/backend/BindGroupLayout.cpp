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

#include "backend/BindGroupLayout.h"

#include "backend/Device.h"

#include <functional>

namespace backend {

    namespace {

        // Workaround for Chrome's stdlib having a broken std::hash for enums and bitsets
        template <typename T>
        typename std::enable_if<std::is_enum<T>::value, size_t>::type Hash(T value) {
            using Integral = typename nxt::UnderlyingType<T>::type;
            return std::hash<Integral>()(static_cast<Integral>(value));
        }

        template <size_t N>
        size_t Hash(const std::bitset<N>& value) {
            static_assert(N <= sizeof(unsigned long long) * 8, "");
            return std::hash<unsigned long long>()(value.to_ullong());
        }

        // TODO(cwallez@chromium.org): see if we can use boost's hash combined or some equivalent
        // this currently assumes that size_t is 64 bits
        void CombineHashes(size_t* h1, size_t h2) {
            *h1 ^= (h2 << 7) + (h2 >> (sizeof(size_t) * 8 - 7)) + 0x304975;
        }

        size_t HashBindingInfo(const BindGroupLayoutBase::LayoutBindingInfo& info) {
            size_t hash = Hash(info.mask);

            for (size_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                if (info.mask[binding]) {
                    CombineHashes(&hash, Hash(info.visibilities[binding]));
                    CombineHashes(&hash, Hash(info.types[binding]));
                }
            }

            return hash;
        }

        bool operator==(const BindGroupLayoutBase::LayoutBindingInfo& a,
                        const BindGroupLayoutBase::LayoutBindingInfo& b) {
            if (a.mask != b.mask) {
                return false;
            }

            for (size_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                if (a.mask[binding]) {
                    if (a.visibilities[binding] != b.visibilities[binding]) {
                        return false;
                    }
                    if (a.types[binding] != b.types[binding]) {
                        return false;
                    }
                }
            }

            return true;
        }
    }  // namespace

    // BindGroupLayoutBase

    BindGroupLayoutBase::BindGroupLayoutBase(BindGroupLayoutBuilder* builder, bool blueprint)
        : mDevice(builder->mDevice), mBindingInfo(builder->mBindingInfo), mIsBlueprint(blueprint) {
    }

    BindGroupLayoutBase::~BindGroupLayoutBase() {
        // Do not register the actual cached object if we are a blueprint
        if (!mIsBlueprint) {
            mDevice->UncacheBindGroupLayout(this);
        }
    }

    const BindGroupLayoutBase::LayoutBindingInfo& BindGroupLayoutBase::GetBindingInfo() const {
        return mBindingInfo;
    }

    DeviceBase* BindGroupLayoutBase::GetDevice() const {
        return mDevice;
    }

    // BindGroupLayoutBuilder

    BindGroupLayoutBuilder::BindGroupLayoutBuilder(DeviceBase* device) : Builder(device) {
    }

    const BindGroupLayoutBase::LayoutBindingInfo& BindGroupLayoutBuilder::GetBindingInfo() const {
        return mBindingInfo;
    }

    BindGroupLayoutBase* BindGroupLayoutBuilder::GetResultImpl() {
        BindGroupLayoutBase blueprint(this, true);

        auto* result = mDevice->GetOrCreateBindGroupLayout(&blueprint, this);
        result->Reference();
        return result;
    }

    void BindGroupLayoutBuilder::SetBindingsType(nxt::ShaderStageBit visibility,
                                                 nxt::BindingType bindingType,
                                                 uint32_t start,
                                                 uint32_t count) {
        if (start + count > kMaxBindingsPerGroup) {
            HandleError("Setting bindings type over maximum number of bindings");
            return;
        }
        for (size_t i = start; i < start + count; i++) {
            if (mBindingInfo.mask[i]) {
                HandleError("Setting already set binding type");
                return;
            }
        }

        for (size_t i = start; i < start + count; i++) {
            mBindingInfo.mask.set(i);
            mBindingInfo.visibilities[i] = visibility;
            mBindingInfo.types[i] = bindingType;
        }
    }

    // BindGroupLayoutCacheFuncs

    size_t BindGroupLayoutCacheFuncs::operator()(const BindGroupLayoutBase* bgl) const {
        return HashBindingInfo(bgl->GetBindingInfo());
    }

    bool BindGroupLayoutCacheFuncs::operator()(const BindGroupLayoutBase* a,
                                               const BindGroupLayoutBase* b) const {
        return a->GetBindingInfo() == b->GetBindingInfo();
    }

}  // namespace backend
