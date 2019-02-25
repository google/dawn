// Copyright 2018 The Dawn Authors
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

#include "dawn_native/Instance.h"

#include "common/Assert.h"
#include "dawn_native/ErrorData.h"

#include <iostream>

namespace dawn_native {

    // Forward definitions of each backend's "Connect" function that creates new BackendConnection.
    // Conditionally compiled declarations are used to avoid using static constructors instead.
#if defined(DAWN_ENABLE_BACKEND_D3D12)
    namespace d3d12 {
        BackendConnection* Connect(InstanceBase* instance);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)
#if defined(DAWN_ENABLE_BACKEND_METAL)
    namespace metal {
        BackendConnection* Connect(InstanceBase* instance);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)
#if defined(DAWN_ENABLE_BACKEND_NULL)
    namespace null {
        BackendConnection* Connect(InstanceBase* instance);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_NULL)
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
    namespace opengl {
        BackendConnection* Connect(InstanceBase* instance);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGL)
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
    namespace vulkan {
        BackendConnection* Connect(InstanceBase* instance);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

    // InstanceBase

    void InstanceBase::DiscoverDefaultAdapters() {
        EnsureBackendConnections();

        if (mDiscoveredDefaultAdapters) {
            return;
        }

        // Query and merge all default adapters for all backends
        for (std::unique_ptr<BackendConnection>& backend : mBackends) {
            std::vector<std::unique_ptr<AdapterBase>> backendAdapters =
                backend->DiscoverDefaultAdapters();

            for (std::unique_ptr<AdapterBase>& adapter : backendAdapters) {
                ASSERT(adapter->GetBackendType() == backend->GetType());
                ASSERT(adapter->GetInstance() == this);
                mAdapters.push_back(std::move(adapter));
            }
        }

        mDiscoveredDefaultAdapters = true;
    }

    // This is just a wrapper around the real logic that uses Error.h error handling.
    bool InstanceBase::DiscoverAdapters(const AdapterDiscoveryOptionsBase* options) {
        return !ConsumedError(DiscoverAdaptersInternal(options));
    }

    const std::vector<std::unique_ptr<AdapterBase>>& InstanceBase::GetAdapters() const {
        return mAdapters;
    }

    void InstanceBase::EnsureBackendConnections() {
        if (mBackendsConnected) {
            return;
        }

        auto Register = [this](BackendConnection* connection, BackendType expectedType) {
            if (connection != nullptr) {
                ASSERT(connection->GetType() == expectedType);
                ASSERT(connection->GetInstance() == this);
                mBackends.push_back(std::unique_ptr<BackendConnection>(connection));
            }
        };

#if defined(DAWN_ENABLE_BACKEND_D3D12)
        Register(d3d12::Connect(this), BackendType::D3D12);
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)
#if defined(DAWN_ENABLE_BACKEND_METAL)
        Register(metal::Connect(this), BackendType::Metal);
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)
#if defined(DAWN_ENABLE_BACKEND_NULL)
        Register(null::Connect(this), BackendType::Null);
#endif  // defined(DAWN_ENABLE_BACKEND_NULL)
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
        Register(opengl::Connect(this), BackendType::OpenGL);
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGL)
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
        Register(vulkan::Connect(this), BackendType::Vulkan);
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

        mBackendsConnected = true;
    }

    ResultOrError<BackendConnection*> InstanceBase::FindBackend(BackendType type) {
        for (std::unique_ptr<BackendConnection>& backend : mBackends) {
            if (backend->GetType() == type) {
                return backend.get();
            }
        }

        return DAWN_VALIDATION_ERROR("Backend isn't present.");
    }

    MaybeError InstanceBase::DiscoverAdaptersInternal(const AdapterDiscoveryOptionsBase* options) {
        EnsureBackendConnections();

        BackendConnection* backend;
        DAWN_TRY_ASSIGN(backend, FindBackend(options->backendType));

        std::vector<std::unique_ptr<AdapterBase>> newAdapters;
        DAWN_TRY_ASSIGN(newAdapters, backend->DiscoverAdapters(options));

        for (std::unique_ptr<AdapterBase>& adapter : newAdapters) {
            ASSERT(adapter->GetBackendType() == backend->GetType());
            ASSERT(adapter->GetInstance() == this);
            mAdapters.push_back(std::move(adapter));
        }

        return {};
    }

    bool InstanceBase::ConsumedError(MaybeError maybeError) {
        if (maybeError.IsError()) {
            ErrorData* error = maybeError.AcquireError();

            ASSERT(error != nullptr);
            std::cout << error->GetMessage() << std::endl;
            delete error;

            return true;
        }
        return false;
    }

}  // namespace dawn_native
