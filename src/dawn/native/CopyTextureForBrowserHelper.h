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

#ifndef SRC_DAWN_NATIVE_COPYTEXTUREFORBROWSERHELPER_H_
#define SRC_DAWN_NATIVE_COPYTEXTUREFORBROWSERHELPER_H_

#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"

namespace dawn::native {
class DeviceBase;
struct Extent3D;
struct ImageCopyTexture;
struct CopyTextureForBrowserOptions;

MaybeError ValidateCopyTextureForBrowser(DeviceBase* device,
                                         const ImageCopyTexture* source,
                                         const ImageCopyTexture* destination,
                                         const Extent3D* copySize,
                                         const CopyTextureForBrowserOptions* options);

MaybeError ValidateCopyExternalTextureForBrowser(DeviceBase* device,
                                                 const ImageCopyExternalTexture* source,
                                                 const ImageCopyTexture* destination,
                                                 const Extent3D* copySize,
                                                 const CopyTextureForBrowserOptions* options);

MaybeError DoCopyTextureForBrowser(DeviceBase* device,
                                   const ImageCopyTexture* source,
                                   const ImageCopyTexture* destination,
                                   const Extent3D* copySize,
                                   const CopyTextureForBrowserOptions* options);

MaybeError DoCopyExternalTextureForBrowser(DeviceBase* device,
                                           const ImageCopyExternalTexture* source,
                                           const ImageCopyTexture* destination,
                                           const Extent3D* copySize,
                                           const CopyTextureForBrowserOptions* options);
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COPYTEXTUREFORBROWSERHELPER_H_
