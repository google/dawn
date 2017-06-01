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

#include "PersistentPipelineStateGL.h"

#include "OpenGLBackend.h"

namespace backend {
namespace opengl {

    PersistentPipelineState::PersistentPipelineState() {
        dirtyFields.set(); // initialize all fields as dirty
    }

    // when a field on PersistentPipelineState::State changes, mark its starting location as dirty
    #define SET_FIELD(field, destination, value) { \
        if (state.destination != value) { \
            dirtyFields.set(field); \
            state.destination = value; \
        } \
    }

    void PersistentPipelineState::UpdateDepthStencilInfo(const DepthStencilStateBase* const depthStencilState) {
        auto& depth = depthStencilState->GetDepth();
        SET_FIELD(DEPTH_COMPARE_FUNCTION, depthInfo.compareFunction, depth.compareFunction)
        SET_FIELD(DEPTH_WRITE_ENABLED, depthInfo.depthWriteEnabled, depth.depthWriteEnabled)
        SET_FIELD(DEPTH_ENABLED, depthEnabled, depthStencilState->DepthTestEnabled())

        auto& stencil = depthStencilState->GetStencil();
        SET_FIELD(STENCIL_ENABLED, stencilEnabled, depthStencilState->StencilTestEnabled())

        SET_FIELD(STENCIL_BACK_COMPARE_FUNCTION, stencilInfo.back.compareFunction, stencil.back.compareFunction)
        SET_FIELD(STENCIL_BACK_STENCIL_FAIL, stencilInfo.back.stencilFail, stencil.back.stencilFail)
        SET_FIELD(STENCIL_BACK_DEPTH_FAIL, stencilInfo.back.depthFail, stencil.back.depthFail)
        SET_FIELD(STENCIL_BACK_DEPTH_STENCIL_PASS, stencilInfo.back.depthStencilPass, stencil.back.depthStencilPass)
        SET_FIELD(STENCIL_BACK_MASK, stencilInfo.back.mask, stencil.back.mask)

        SET_FIELD(STENCIL_FRONT_COMPARE_FUNCTION, stencilInfo.front.compareFunction, stencil.front.compareFunction)
        SET_FIELD(STENCIL_FRONT_STENCIL_FAIL, stencilInfo.front.stencilFail, stencil.front.stencilFail)
        SET_FIELD(STENCIL_FRONT_DEPTH_FAIL, stencilInfo.front.depthFail, stencil.front.depthFail)
        SET_FIELD(STENCIL_FRONT_DEPTH_STENCIL_PASS, stencilInfo.front.depthStencilPass, stencil.front.depthStencilPass)
        SET_FIELD(STENCIL_FRONT_MASK, stencilInfo.front.mask, stencil.front.mask)
    }

    void PersistentPipelineState::UpdateStencilReference(uint32_t stencilReference) {
        SET_FIELD(STENCIL_REFERENCE, stencilReference, stencilReference)
    }

    #undef SET_FIELD

    bool PersistentPipelineState::IsDirty(Field field) const {
        return dirtyFields.test(field);
    }

    void PersistentPipelineState::CleanField(Field field) {
        dirtyFields.reset(field);
    }

    void PersistentPipelineState::ApplyDepthNow() {
        if (IsDirty(DEPTH_ENABLED)) {
            if (state.depthEnabled) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
            CleanField(DEPTH_ENABLED);
        }
        if (IsDirty(DEPTH_WRITE_ENABLED)) {
            if (state.depthInfo.depthWriteEnabled) {
                glDepthMask(GL_TRUE);
            } else {
                glDepthMask(GL_FALSE);
            }
            CleanField(DEPTH_WRITE_ENABLED);
        }
        if (IsDirty(DEPTH_COMPARE_FUNCTION)) {
            glDepthFunc(OpenGLCompareFunction(state.depthInfo.compareFunction));
            CleanField(DEPTH_COMPARE_FUNCTION);
        }
    }

    void PersistentPipelineState::ApplyStencilNow() {
        if (IsDirty(STENCIL_ENABLED)) {
            if (state.stencilEnabled) {
                glEnable(GL_STENCIL_TEST);
            } else {
                glDisable(GL_STENCIL_TEST);
            }
            CleanField(STENCIL_ENABLED);
        }

        if (IsDirty(STENCIL_BACK_STENCIL_FAIL) ||
            IsDirty(STENCIL_BACK_DEPTH_FAIL) ||
            IsDirty(STENCIL_BACK_DEPTH_STENCIL_PASS)) {

            glStencilOpSeparate(GL_BACK,
                OpenGLStencilOperation(state.stencilInfo.back.stencilFail),
                OpenGLStencilOperation(state.stencilInfo.back.depthFail),
                OpenGLStencilOperation(state.stencilInfo.back.depthStencilPass)
            );

            CleanField(STENCIL_BACK_STENCIL_FAIL);
            CleanField(STENCIL_BACK_DEPTH_FAIL);
            CleanField(STENCIL_BACK_DEPTH_STENCIL_PASS);
        }
        if (IsDirty(STENCIL_BACK_COMPARE_FUNCTION) ||
            IsDirty(STENCIL_REFERENCE) ||
            IsDirty(STENCIL_BACK_MASK)) {

            glStencilFuncSeparate(GL_BACK,
                OpenGLCompareFunction(state.stencilInfo.back.compareFunction),
                state.stencilReference,
                state.stencilInfo.back.mask
            );
            if (IsDirty(STENCIL_BACK_MASK)) {
                glStencilMaskSeparate(GL_BACK, state.stencilInfo.back.mask);
            }

            CleanField(STENCIL_BACK_COMPARE_FUNCTION);
            CleanField(STENCIL_BACK_MASK);
        }

        if (IsDirty(STENCIL_FRONT_STENCIL_FAIL) ||
            IsDirty(STENCIL_FRONT_DEPTH_FAIL) ||
            IsDirty(STENCIL_FRONT_DEPTH_STENCIL_PASS)) {

            glStencilOpSeparate(GL_FRONT,
                OpenGLStencilOperation(state.stencilInfo.front.stencilFail),
                OpenGLStencilOperation(state.stencilInfo.front.depthFail),
                OpenGLStencilOperation(state.stencilInfo.front.depthStencilPass)
            );

            CleanField(STENCIL_FRONT_STENCIL_FAIL);
            CleanField(STENCIL_FRONT_DEPTH_FAIL);
            CleanField(STENCIL_FRONT_DEPTH_STENCIL_PASS);
        }
        if (IsDirty(STENCIL_FRONT_COMPARE_FUNCTION) ||
            IsDirty(STENCIL_REFERENCE) ||
            IsDirty(STENCIL_FRONT_MASK)) {

            glStencilFuncSeparate(GL_FRONT,
                OpenGLCompareFunction(state.stencilInfo.front.compareFunction),
                state.stencilReference,
                state.stencilInfo.front.mask
            );
            if (IsDirty(STENCIL_FRONT_MASK)) {
                glStencilMaskSeparate(GL_FRONT, state.stencilInfo.front.mask);
            }

            CleanField(STENCIL_FRONT_COMPARE_FUNCTION);
            CleanField(STENCIL_FRONT_MASK);
        }

        CleanField(STENCIL_REFERENCE); // clean this last because its used for both the back and front functions
    }

}
}