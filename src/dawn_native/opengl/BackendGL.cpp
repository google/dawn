// Copyright 2019 The Dawn Authors
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

#include "dawn_native/opengl/BackendGL.h"

#include "dawn_native/OpenGLBackend.h"
#include "dawn_native/opengl/DeviceGL.h"

namespace dawn_native { namespace opengl {

    // The OpenGL backend's Adapter.

    class Adapter : public AdapterBase {
      public:
        Adapter(InstanceBase* instance, const AdapterDiscoveryOptions* options)
            : AdapterBase(instance, BackendType::OpenGL) {
            // Use getProc to populate GLAD.
            gladLoadGLLoader(reinterpret_cast<GLADloadproc>(options->getProc));

            // Set state that never changes between devices.
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_SCISSOR_TEST);
            glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        }
        virtual ~Adapter() = default;

      private:
        ResultOrError<DeviceBase*> CreateDeviceImpl() override {
            // There is no limit on the number of devices created from this adapter because they can
            // all share the same backing OpenGL context.
            return {new Device};
        }
    };

    // Implementation of the OpenGL backend's BackendConnection

    Backend::Backend(InstanceBase* instance) : BackendConnection(instance, BackendType::OpenGL) {
    }

    std::vector<std::unique_ptr<AdapterBase>> Backend::DiscoverDefaultAdapters() {
        // The OpenGL backend needs at least "getProcAddress" to discover an adapter.
        return {};
    }

    ResultOrError<std::vector<std::unique_ptr<AdapterBase>>> Backend::DiscoverAdapters(
        const AdapterDiscoveryOptionsBase* optionsBase) {
        // TODO(cwallez@chromium.org): For now we can only create a single adapter because glad uses
        // static variables to store the pointers to OpenGL procs. Also, if we could create
        // multiple adapters, we would need to figure out what to do about MakeCurrent.
        if (mCreatedAdapter) {
            return DAWN_VALIDATION_ERROR("The OpenGL backend can only create a single adapter");
        }

        ASSERT(optionsBase->backendType == BackendType::OpenGL);
        const AdapterDiscoveryOptions* options =
            reinterpret_cast<const AdapterDiscoveryOptions*>(optionsBase);

        if (options->getProc == nullptr) {
            return DAWN_VALIDATION_ERROR("AdapterDiscoveryOptions::getProc must be set");
        }

        std::vector<std::unique_ptr<AdapterBase>> adapters;
        adapters.push_back(std::make_unique<Adapter>(GetInstance(), options));

        mCreatedAdapter = true;
        return std::move(adapters);
    }

    BackendConnection* Connect(InstanceBase* instance) {
        return new Backend(instance);
    }

}}  // namespace dawn_native::opengl
