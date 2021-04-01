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

#ifndef DAWNNATIVE_EXTERNALTEXTURE_H_
#define DAWNNATIVE_EXTERNALTEXTURE_H_

#include "dawn_native/Error.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/Subresource.h"

#include <array>

namespace dawn_native {

    struct ExternalTextureDescriptor;
    class TextureViewBase;

    MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                                 const ExternalTextureDescriptor* descriptor);

    class ExternalTextureBase : public ObjectBase {
      public:
        static ResultOrError<Ref<ExternalTextureBase>> Create(
            DeviceBase* device,
            const ExternalTextureDescriptor* descriptor);

        std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat> GetTextureViews() const;

        static ExternalTextureBase* MakeError(DeviceBase* device);

        void APIDestroy();

      private:
        ExternalTextureBase(DeviceBase* device, const ExternalTextureDescriptor* descriptor);
        ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag);
        std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat> textureViews;
    };
}  // namespace dawn_native

#endif  // DAWNNATIVE_EXTERNALTEXTURE_H_