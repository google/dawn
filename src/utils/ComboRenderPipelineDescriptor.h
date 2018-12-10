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

#ifndef UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_
#define UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_

#include <dawn/dawncpp.h>

#include "common/Constants.h"

namespace utils {

    class ComboRenderPipelineDescriptor : public dawn::RenderPipelineDescriptor {
      public:
        ComboRenderPipelineDescriptor(const dawn::Device& device);

        dawn::PipelineStageDescriptor cVertexStage;
        dawn::PipelineStageDescriptor cFragmentStage;

        dawn::AttachmentsStateDescriptor cAttachmentsState;
        dawn::AttachmentDescriptor cColorAttachments[kMaxColorAttachments];
        dawn::AttachmentDescriptor cDepthStencilAttachment;
        dawn::BlendState cBlendStates[kMaxColorAttachments];
    };

}  // namespace utils

#endif  // UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_