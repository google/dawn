// Copyright 2021 The Dawn & Tint Authors
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
    ~RequestTracker() { DAWN_ASSERT(mRequests.empty()); }

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
