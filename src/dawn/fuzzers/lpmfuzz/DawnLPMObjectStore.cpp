// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include <functional>
#include <limits>
#include <utility>

#include "dawn/fuzzers/lpmfuzz/DawnLPMConstants_autogen.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMObjectStore.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h"
#include "dawn/wire/ObjectHandle.h"

namespace dawn::wire {

DawnLPMObjectStore::DawnLPMObjectStore() {
    mCurrentId = 1;
}

ObjectHandle DawnLPMObjectStore::ReserveHandle() {
    if (mFreeHandles.empty()) {
        Insert(mCurrentId);
        return {mCurrentId++, 0};
    }
    ObjectHandle handle = mFreeHandles.back();
    mFreeHandles.pop_back();
    Insert(handle.id);
    return handle;
}

void DawnLPMObjectStore::Insert(ObjectId id) {
    std::vector<ObjectId>::iterator iter =
        std::lower_bound(mObjects.begin(), mObjects.end(), id, std::greater<ObjectId>());
    mObjects.insert(iter, id);
}

void DawnLPMObjectStore::Free(ObjectId id) {
    if (id == DawnLPMFuzzer::kInvalidObjectId) {
        return;
    }

    for (size_t i = 0; i < mObjects.size(); i++) {
        if (mObjects[i] == id) {
            mFreeHandles.push_back({id, 0});
            mObjects.erase(mObjects.begin() + i);
        }
    }
}

/*
 * Consistent hashing inspired map for fuzzer state.
 * If we store the Dawn objects in a hash table mapping FuzzInt -> ObjectId
 * then it would be highly unlikely that any subsequence DestroyObject command
 * would come up with an ID that would correspond to a valid ObjectId in the
 * hash table.
 *
 * One solution is to modulo the FuzzInt with the length of the hash table, but
 * it does not work well with libfuzzer's minimization techniques because
 * deleting a single ObjectId from the hash table changes the index of every
 * entry from then on.
 *
 * So we use consistent hashing. we take the entry in the table that
 * has the next highest id (wrapping when there is no higher entry).
 */
ObjectId DawnLPMObjectStore::Lookup(uint32_t id) const {
    // CreateBindGroup relies on sending invalid object ids
    if (id == DawnLPMFuzzer::kInvalidObjectId) {
        return 0;
    }

    auto iter = std::lower_bound(mObjects.begin(), mObjects.end(), id, std::greater<ObjectId>());
    if (iter != mObjects.end()) {
        return *iter;
    }

    // Wrap to 0
    iter = std::lower_bound(mObjects.begin(), mObjects.end(), 0, std::greater<ObjectId>());
    if (iter != mObjects.end()) {
        return *iter;
    }

    return 0;
}

size_t DawnLPMObjectStore::Size() const {
    return mObjects.size();
}

}  // namespace dawn::wire
