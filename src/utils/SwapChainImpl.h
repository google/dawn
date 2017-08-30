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

#ifndef UTILS_SWAPCHAINIMPL_H_
#define UTILS_SWAPCHAINIMPL_H_

namespace utils {
    class SwapChainImpl {
        protected:
            template<class TImpl, typename TWSIContext>
            static nxtSwapChainImplementation GenerateSwapChainImplementation() {
                nxtSwapChainImplementation impl = {};
                impl.Init = [](void* userData, void* wsiContext) {
                    auto* ctx = reinterpret_cast<TWSIContext*>(wsiContext);
                    reinterpret_cast<TImpl*>(userData)->Init(ctx);
                };
                impl.Destroy = [](void* userData) {
                    delete reinterpret_cast<TImpl*>(userData);
                };
                impl.Configure = [](void* userData, nxtTextureFormat format, nxtTextureUsageBit allowedUsage, uint32_t width, uint32_t height) {
                    return reinterpret_cast<TImpl*>(userData)->Configure(format, allowedUsage, width, height);
                };
                impl.GetNextTexture = [](void* userData, nxtSwapChainNextTexture* nextTexture) {
                    return reinterpret_cast<TImpl*>(userData)->GetNextTexture(
                        nextTexture);
                };
                impl.Present = [](void* userData) {
                    return reinterpret_cast<TImpl*>(userData)->Present();
                };
                return impl;
            }
    };
}

#endif // UTILS_SWAPCHAINIMPL_H_
