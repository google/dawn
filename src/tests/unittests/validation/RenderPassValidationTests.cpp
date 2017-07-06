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

#include "tests/unittests/validation/ValidationTest.h"

class RenderPassValidationTest : public ValidationTest {
};

// Test for an empty render pass builder
TEST_F(RenderPassValidationTest, Empty) {
    AssertWillBeError(device.CreateRenderPassBuilder())
        .GetResult();
}

// Test for a render pass with one subpass and no attachments
TEST_F(RenderPassValidationTest, OneSubpass) {
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(0)
        .GetResult();
}

// Test for a render pass with one subpass and one attachment
TEST_F(RenderPassValidationTest, OneSubpassOneAttachment) {
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
}
