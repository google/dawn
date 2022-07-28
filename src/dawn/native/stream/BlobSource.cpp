// Copyright 2022 The Dawn Authors
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

#include "dawn/native/stream/BlobSource.h"

#include <utility>

namespace dawn::native::stream {

BlobSource::BlobSource(Blob&& blob) : mBlob(std::move(blob)) {}

MaybeError BlobSource::Read(const void** ptr, size_t bytes) {
    DAWN_INVALID_IF(bytes > mBlob.Size() - mOffset, "Out of bounds.");
    *ptr = mBlob.Data() + mOffset;
    mOffset += bytes;
    return {};
}

}  // namespace dawn::native::stream
