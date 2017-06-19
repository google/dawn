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

#include "SampleUtils.h"

#include "utils/NXTHelpers.h"

#include <cstdlib>
#include <cstdio>
#include <vector>

nxt::Device device;
nxt::Queue queue;
nxt::Pipeline pipeline;
nxt::RenderPass renderpass;
nxt::Framebuffer framebuffer;

float RandomFloat(float min, float max) {
    float zeroOne = rand() / float(RAND_MAX);
    return zeroOne * (max - min) + min;
}

struct ShaderData {
    float scale;
    float time;
    float offsetX;
    float offsetY;
    float scalar;
    float scalarOffset;
};

static std::vector<ShaderData> shaderData;

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueueBuilder().GetResult();

    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450

        layout(push_constant) uniform ConstantsBlock {
            float scale;
            float time;
            float offsetX;
            float offsetY;
            float scalar;
            float scalarOffset;
        } c;

        layout(location = 0) out vec4 v_color;

        const vec4 positions[3] = vec4[3](
            vec4( 0.0f,  0.1f, 0.0f, 1.0f),
            vec4(-0.1f, -0.1f, 0.0f, 1.0f),
            vec4( 0.1f, -0.1f, 0.0f, 1.0f)
        );

        const vec4 colors[3] = vec4[3](
            vec4(1.0f, 0.0f, 0.0f, 1.0f),
            vec4(0.0f, 1.0f, 0.0f, 1.0f),
            vec4(0.0f, 0.0f, 1.0f, 1.0f)
        );

        void main() {
            vec4 position = positions[gl_VertexIndex];
            vec4 color = colors[gl_VertexIndex];

            float fade = mod(c.scalarOffset + c.time * c.scalar / 10.0, 1.0);
            if (fade < 0.5) {
                fade = fade * 2.0;
            } else {
                fade = (1.0 - fade) * 2.0;
            }
            float xpos = position.x * c.scale;
            float ypos = position.y * c.scale;
            float angle = 3.14159 * 2.0 * fade;
            float xrot = xpos * cos(angle) - ypos * sin(angle);
            float yrot = xpos * sin(angle) + ypos * cos(angle);
            xpos = xrot + c.offsetX;
            ypos = yrot + c.offsetY;
            v_color = vec4(fade, 1.0 - fade, 0.0, 1.0) + color;
            gl_Position = vec4(xpos, ypos, 0.0, 1.0);
        })"
    );

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        out vec4 fragColor;
        layout(location = 0) in vec4 v_color;
        void main() {
            fragColor = v_color;
        })"
    );

    utils::CreateDefaultRenderPass(device, &renderpass, &framebuffer);
    pipeline = device.CreatePipelineBuilder()
        .SetSubpass(renderpass, 0)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .GetResult();

    shaderData.resize(10000);
    for (auto& data : shaderData) {
        data.scale = RandomFloat(0.2, 0.4);
        data.time = 0.0;
        data.offsetX = RandomFloat(-0.9, 0.9);
        data.offsetY = RandomFloat(-0.9, 0.9);
        data.scalar = RandomFloat(0.5, 2.0);
        data.scalarOffset = RandomFloat(0.0, 10.0);
    }
}

void frame() {
    static int f = 0;
    f++;

    size_t i = 0;

    std::vector<nxt::CommandBuffer> commands(50);
    for (int j = 0; j < 50; j++) {

        nxt::CommandBufferBuilder builder = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderpass, framebuffer)
            .SetPipeline(pipeline)
            .Clone();

        for (int k = 0; k < 200; k++) {

            shaderData[i].time = f / 60.0f;
            builder.SetPushConstants(nxt::ShaderStageBit::Vertex, 0, 6, reinterpret_cast<uint32_t*>(&shaderData[i]))
                   .DrawArrays(3, 1, 0, 0);
            i++;
        }

        builder.EndRenderPass();
        commands[j] = builder.GetResult();
    }

    queue.Submit(50, commands.data());
    DoSwapBuffers();
    fprintf(stderr, "frame %i\n", f);
}

int main(int argc, const char* argv[]) {
    if (!InitSample(argc, argv)) {
        return 1;
    }
    init();

    while (!ShouldQuit()) {
        frame();
        USleep(16000);
    }

    // TODO release stuff
}
