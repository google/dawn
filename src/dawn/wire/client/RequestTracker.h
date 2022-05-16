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

#ifndef SRC_DAWN_WIRE_CLIENT_REQUESTTRACKER_H_
#define SRC_DAWN_WIRE_CLIENT_REQUESTTRACKER_H_

#include <cstdint>
#include <map>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/NonCopyable.h"

namespace dawn::wire::client {

class Device;
class MemoryTransferService;

template <typename Request>
class RequestTracker : NonCopyable {
  public:
    ~RequestTracker() { ASSERT(mRequests.empty()); }

    uint64_t Add(Request&& request) {
        mSerial++;
        mRequests.emplace(mSerial, request);
        return mSerial;
    }

    bool Acquire(uint64_t serial, Request* request) {
        auto it = mRequests.find(serial);
        if (it == mRequests.end()) {
            return false;
        }
        *request = std::move(it->second);
        mRequests.erase(it);
        return true;
    }

    template <typename CloseFunc>
    void CloseAll(CloseFunc&& closeFunc) {
        // Call closeFunc on all requests while handling reentrancy where the callback of some
        // requests may add some additional requests. We guarantee all callbacks for requests
        // are called exactly onces, so keep closing new requests if the first batch added more.
        // It is fine to loop infinitely here if that's what the application makes use do.
        while (!mRequests.empty()) {
            // Move mRequests to a local variable so that further reentrant modifications of
            // mRequests don't invalidate the iterators.
            auto allRequests = std::move(mRequests);
            for (auto& [_, request] : allRequests) {
                closeFunc(&request);
            }
        }
    }

    template <typename F>
    void ForAll(F&& f) {
        for (auto& [_, request] : mRequests) {
            f(&request);
        }
    }

  private:
    uint64_t mSerial = 0;
    std::map<uint64_t, Request> mRequests;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_REQUESTTRACKER_H_
