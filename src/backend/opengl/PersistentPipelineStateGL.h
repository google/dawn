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

#ifndef BACKEND_OPENGL_PERSISTENTPIPELINESTATE_H_
#define BACKEND_OPENGL_PERSISTENTPIPELINESTATE_H_

#include "common/DepthStencilState.h"

#include <bitset>

namespace backend {
namespace opengl {

    class PersistentPipelineState {
        public:
            PersistentPipelineState();
            void UpdateDepthStencilInfo(const DepthStencilStateBase* const depthStencilState);
            void UpdateStencilReference(uint32_t stencilReference);

            void ApplyDepthNow();
            void ApplyStencilNow();

            enum Field {
                DEPTH_COMPARE_FUNCTION,
                DEPTH_WRITE_ENABLED,
                DEPTH_ENABLED,
                STENCIL_ENABLED,
                STENCIL_BACK_COMPARE_FUNCTION,
                STENCIL_BACK_STENCIL_FAIL,
                STENCIL_BACK_DEPTH_FAIL,
                STENCIL_BACK_DEPTH_STENCIL_PASS,
                STENCIL_BACK_MASK,
                STENCIL_FRONT_COMPARE_FUNCTION,
                STENCIL_FRONT_STENCIL_FAIL,
                STENCIL_FRONT_DEPTH_FAIL,
                STENCIL_FRONT_DEPTH_STENCIL_PASS,
                STENCIL_FRONT_MASK,
                STENCIL_REFERENCE,
                Count
            };

            struct State {
                bool depthEnabled;
                bool stencilEnabled;
                DepthStencilStateBase::DepthInfo depthInfo;
                DepthStencilStateBase::StencilInfo stencilInfo;
                uint32_t stencilReference;
            };

        private:
            State state;
            std::bitset<Field::Count> dirtyFields;

            inline bool IsDirty(Field field) const;
            inline void CleanField(Field field);
    };

}
}

#endif // BACKEND_OPENGL_PERSISTENTPIPELINESTATE_H_