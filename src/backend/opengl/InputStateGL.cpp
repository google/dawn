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

#include "backend/opengl/InputStateGL.h"

#include "common/Assert.h"

namespace backend { namespace opengl {

    InputState::InputState(InputStateBuilder* builder) : InputStateBase(builder) {
        glGenVertexArrays(1, &mVertexArrayObject);
        glBindVertexArray(mVertexArrayObject);
        auto& attributesSetMask = GetAttributesSetMask();
        for (uint32_t location = 0; location < attributesSetMask.size(); ++location) {
            if (!attributesSetMask[location]) {
                continue;
            }
            auto attribute = GetAttribute(location);
            glEnableVertexAttribArray(location);

            attributesUsingInput[attribute.bindingSlot][location] = true;
            auto input = GetInput(attribute.bindingSlot);

            if (input.stride == 0) {
                // Emulate a stride of zero (constant vertex attribute) by
                // setting the attribute instance divisor to a huge number.
                glVertexAttribDivisor(location, 0xffffffff);
            } else {
                switch (input.stepMode) {
                    case nxt::InputStepMode::Vertex:
                        break;
                    case nxt::InputStepMode::Instance:
                        glVertexAttribDivisor(location, 1);
                        break;
                    default:
                        UNREACHABLE();
                }
            }
        }
    }

    std::bitset<kMaxVertexAttributes> InputState::GetAttributesUsingInput(uint32_t slot) const {
        return attributesUsingInput[slot];
    }

    GLuint InputState::GetVAO() {
        return mVertexArrayObject;
    }

}}  // namespace backend::opengl
