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

#ifndef BACKEND_D3D12_TEXTURED3D12_H_
#define BACKEND_D3D12_TEXTURED3D12_H_

#include "backend/Texture.h"

#include "backend/d3d12/d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class Device;

    DXGI_FORMAT D3D12TextureFormat(nxt::TextureFormat format);

    class Texture : public TextureBase {
        public:
            Texture(TextureBuilder* builder);
            Texture(TextureBuilder* builder, ID3D12Resource* nativeTexture);
            ~Texture();

            DXGI_FORMAT GetD3D12Format() const;
            ID3D12Resource* GetD3D12Resource();
            bool GetResourceTransitionBarrier(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage, D3D12_RESOURCE_BARRIER* barrier);

        private:
            Device* device;
            ComPtr<ID3D12Resource> resource = {};
            ID3D12Resource* resourcePtr = nullptr;

            // NXT API
            void TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) override;
    };

    class TextureView : public TextureViewBase {
        public:
            TextureView(TextureViewBuilder* builder);

            const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDescriptor() const;
            D3D12_RENDER_TARGET_VIEW_DESC  GetRTVDescriptor();
            D3D12_DEPTH_STENCIL_VIEW_DESC  GetDSVDescriptor();

        private:
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    };
}
}

#endif // BACKEND_D3D12_TEXTURED3D12_H_
