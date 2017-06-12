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

// Enable this before including any headers as we want inttypes.h to define
// format macros such as PRId64 that are used in picojson.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "Utils.h"

#include <bitset>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLFW/glfw3.h"
#define TINYGLTF_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tinygltfloader/tiny_gltf_loader.h>

#include "Camera.inl"

namespace gl {
    enum {
        Triangles = 0x0004,
        UnsignedShort = 0x1403,
        UnsignedInt = 0x1405,
        Float = 0x1406,
        RGBA = 0x1908,
        Nearest = 0x2600,
        Linear = 0x2601,
        NearestMipmapNearest = 0x2700,
        LinearMipmapNearest = 0x2701,
        NearestMipmapLinear = 0x2702,
        LinearMipmapLinear = 0x2703,
        ArrayBuffer = 0x8892,
        ElementArrayBuffer = 0x8893,
        FragmentShader = 0x8B30,
        VertexShader = 0x8B31,
        FloatVec2 = 0x8B50,
        FloatVec3 = 0x8B51,
        FloatVec4 = 0x8B52,
    };
}

struct MaterialInfo {
    nxt::Buffer uniformBuffer;
    nxt::Pipeline pipeline;
    nxt::BindGroup bindGroup0;
    std::map<uint32_t, std::string> slotSemantics;
};

struct u_transform_block {
    glm::mat4 modelViewProj;
    glm::mat4 modelInvTr;
};

nxt::Device device;
nxt::Queue queue;
nxt::RenderPass renderpass;
nxt::Framebuffer framebuffer;

nxt::Buffer defaultBuffer;
std::map<std::string, nxt::Buffer> buffers;
std::map<std::string, nxt::CommandBuffer> commandBuffers;
std::map<uint32_t, std::string> slotSemantics = {{0, "POSITION"}, {1, "NORMAL"}, {2, "TEXCOORD_0"}};

nxt::Sampler defaultSampler;
std::map<std::string, nxt::Sampler> samplers;
nxt::TextureView defaultTexture;
std::map<std::string, nxt::TextureView> textures;

tinygltf::Scene scene;
glm::mat4 projection = glm::perspective(glm::radians(60.f), 640.f/480, 0.1f, 2000.f);
Camera camera;

// Helpers
namespace {
    std::string getFilePathExtension(const std::string &FileName) {
        if (FileName.find_last_of(".") != std::string::npos) {
            return FileName.substr(FileName.find_last_of(".") + 1);
        }
        return "";
    }

    bool techniqueParameterTypeToVertexFormat(int type, nxt::VertexFormat *format) {
        switch (type) {
            case gl::FloatVec2:
                *format = nxt::VertexFormat::FloatR32G32;
                return true;
            case gl::FloatVec3:
                *format = nxt::VertexFormat::FloatR32G32B32;
                return true;
            case gl::FloatVec4:
                *format = nxt::VertexFormat::FloatR32G32B32A32;
                return true;
            default:
                return false;
        }
    }
}

// Initialization
namespace {
    void initBuffers() {
        defaultBuffer = device.CreateBufferBuilder()
            .SetAllowedUsage(nxt::BufferUsageBit::Vertex | nxt::BufferUsageBit::Index)
            .SetSize(256)
            .GetResult();
        defaultBuffer.FreezeUsage(nxt::BufferUsageBit::Vertex | nxt::BufferUsageBit::Index);

        for (const auto& bv : scene.bufferViews) {
            const auto& iBufferViewID = bv.first;
            const auto& iBufferView = bv.second;

            nxt::BufferUsageBit usage = nxt::BufferUsageBit::None;
            switch (iBufferView.target) {
                case gl::ArrayBuffer:
                    usage |= nxt::BufferUsageBit::Vertex;
                    break;
                case gl::ElementArrayBuffer:
                    usage |= nxt::BufferUsageBit::Index;
                    break;
                case 0:
                    fprintf(stderr, "TODO: buffer view has no target; skipping\n");
                    continue;
                default:
                    fprintf(stderr, "unsupported buffer view target %d\n", iBufferView.target);
                    continue;
            }
            const auto& iBuffer = scene.buffers.at(iBufferView.buffer);

            uint32_t iBufferViewSize =
                iBufferView.byteLength ? iBufferView.byteLength :
                (iBuffer.data.size() - iBufferView.byteOffset);
            auto oBuffer = CreateFrozenBufferFromData(device, &iBuffer.data.at(iBufferView.byteOffset), iBufferViewSize, usage);
            buffers[iBufferViewID] = std::move(oBuffer);
        }
    }

    const MaterialInfo& getMaterial(const std::string& iMaterialID, uint32_t stridePos, uint32_t strideNor, uint32_t strideTxc) {
        static std::map<std::tuple<std::string, uint32_t, uint32_t, uint32_t>, MaterialInfo> materials;
        auto key = make_tuple(iMaterialID, stridePos, strideNor, strideTxc);
        auto it = materials.find(key);
        if (it != materials.end()) {
            return it->second;
        }

        const auto& iMaterial = scene.materials.at(iMaterialID);
        const auto& iTechnique = scene.techniques.at(iMaterial.technique);
        const auto& iProgram = scene.programs.at(iTechnique.program);

        auto oVSModule = CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
            #version 450

            layout(set = 0, binding = 0) uniform u_transform_block {
                mat4 modelViewProj;
                mat4 modelInvTr;
            } u_transform;

            layout(location = 0) in vec4 a_position;
            layout(location = 1) in vec3 a_normal;
            layout(location = 2) in vec2 a_texcoord;

            layout(location = 0) out vec3 v_normal;
            layout(location = 1) out vec2 v_texcoord;

            void main() {
                v_normal = (u_transform.modelInvTr * vec4(normalize(a_normal), 0)).rgb;
                v_texcoord = a_texcoord;
                gl_Position = u_transform.modelViewProj * a_position;
            })");

        auto oFSModule = CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
            #version 450

            layout(set = 0, binding = 1) uniform sampler u_samp;
            layout(set = 0, binding = 2) uniform texture2D u_tex;

            layout(location = 0) in vec3 v_normal;
            layout(location = 1) in vec2 v_texcoord;

            out vec4 fragcolor;

            void main() {
                const vec3 lightdir = normalize(vec3(-1, -2, 3));
                vec3 normal = normalize(v_normal);
                float diffuse = abs(dot(lightdir, normal));
                float diffamb = diffuse * 0.85 + 0.15;
                vec3 albedo = texture(sampler2D(u_tex, u_samp), v_texcoord).rgb;
                fragcolor = vec4(diffamb * albedo, 1);
            })");

        nxt::InputStateBuilder builder = device.CreateInputStateBuilder();
        std::bitset<3> slotsSet;
        for (const auto& a : iTechnique.attributes) {
            const auto iAttributeName = a.first;
            const auto iParameter = iTechnique.parameters.at(a.second);
            nxt::VertexFormat format;
            if (!techniqueParameterTypeToVertexFormat(iParameter.type, &format)) {
                fprintf(stderr, "unsupported technique parameter type %d\n", iParameter.type);
                continue;
            }
            if (iParameter.semantic == "POSITION") {
                builder.SetAttribute(0, 0, format, 0);
                builder.SetInput(0, stridePos, nxt::InputStepMode::Vertex);
                slotsSet.set(0);
            } else if (iParameter.semantic == "NORMAL") {
                builder.SetAttribute(1, 1, format, 0);
                builder.SetInput(1, strideNor, nxt::InputStepMode::Vertex);
                slotsSet.set(1);
            } else if (iParameter.semantic == "TEXCOORD_0") {
                builder.SetAttribute(2, 2, format, 0);
                builder.SetInput(2, strideTxc, nxt::InputStepMode::Vertex);
                slotsSet.set(2);
            } else {
                fprintf(stderr, "unsupported technique attribute semantic %s\n", iParameter.semantic.c_str());
            }
            // TODO: use iAttributeParameter.node?
        }
        for (size_t i = 0; i < slotsSet.size(); i++) {
            if (slotsSet[i]) {
                continue;
            }
            builder.SetAttribute(i, i, nxt::VertexFormat::FloatR32G32B32A32, 0);
            builder.SetInput(i, 0, nxt::InputStepMode::Vertex);
        }
        auto inputState = builder.GetResult();

        auto bindGroupLayout = device.CreateBindGroupLayoutBuilder()
            .SetBindingsType(nxt::ShaderStageBit::Vertex, nxt::BindingType::UniformBuffer, 0, 1)
            .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::Sampler, 1, 1)
            .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::SampledTexture, 2, 1)
            .GetResult();

        auto depthStencilState = device.CreateDepthStencilStateBuilder()
            .SetDepthCompareFunction(nxt::CompareFunction::Less)
            .SetDepthWriteEnabled(true)
            .GetResult();

        auto pipelineLayout = device.CreatePipelineLayoutBuilder()
            .SetBindGroupLayout(0, bindGroupLayout)
            .GetResult();
        auto pipeline = device.CreatePipelineBuilder()
            .SetSubpass(renderpass, 0)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, oVSModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, oFSModule, "main")
            .SetInputState(inputState)
            .SetDepthStencilState(depthStencilState)
            .GetResult();

        auto uniformBuffer = device.CreateBufferBuilder()
            .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::Uniform)
            .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
            .SetSize(sizeof(u_transform_block))
            .GetResult();

        auto uniformView = uniformBuffer.CreateBufferViewBuilder()
            .SetExtent(0, sizeof(u_transform_block))
            .GetResult();

        auto bindGroupBuilder = device.CreateBindGroupBuilder();
        bindGroupBuilder.SetLayout(bindGroupLayout)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &uniformView);
        {
            auto it = iMaterial.values.find("diffuse");
            if (it != iMaterial.values.end() && !it->second.string_value.empty()) {
                const auto& iTextureID = it->second.string_value;
                const auto& textureView = textures[iTextureID];
                const auto& iSamplerID = scene.textures[iTextureID].sampler;
                bindGroupBuilder.SetSamplers(1, 1, &samplers[iSamplerID]);
                bindGroupBuilder.SetTextureViews(2, 1, &textureView);
            } else {
                bindGroupBuilder.SetSamplers(1, 1, &defaultSampler);
                bindGroupBuilder.SetTextureViews(2, 1, &defaultTexture);
            }
        }

        MaterialInfo material = {
            uniformBuffer.Get(),
            pipeline.Get(),
            bindGroupBuilder.GetResult(),
            std::map<uint32_t, std::string>(),
        };
        materials[key] = std::move(material);
        return materials.at(key);
    }

    void initSamplers() {
        defaultSampler = device.CreateSamplerBuilder()
            .SetFilterMode(nxt::FilterMode::Nearest, nxt::FilterMode::Nearest, nxt::FilterMode::Nearest)
            // TODO: wrap modes
            .GetResult();

        for (const auto& s : scene.samplers) {
            const auto& iSamplerID = s.first;
            const auto& iSampler = s.second;

            auto magFilter = nxt::FilterMode::Nearest;
            auto minFilter = nxt::FilterMode::Nearest;
            auto mipmapFilter = nxt::FilterMode::Nearest;
            switch (iSampler.magFilter) {
                case gl::Nearest:
                    magFilter = nxt::FilterMode::Nearest;
                    break;
                case gl::Linear:
                    magFilter = nxt::FilterMode::Linear;
                    break;
                default:
                    fprintf(stderr, "unsupported magFilter %d\n", iSampler.magFilter);
                    break;
            }
            switch (iSampler.minFilter) {
                case gl::Nearest:
                case gl::NearestMipmapNearest:
                case gl::NearestMipmapLinear:
                    minFilter = nxt::FilterMode::Nearest;
                    break;
                case gl::Linear:
                case gl::LinearMipmapNearest:
                case gl::LinearMipmapLinear:
                    minFilter = nxt::FilterMode::Linear;
                    break;
                default:
                    fprintf(stderr, "unsupported minFilter %d\n", iSampler.magFilter);
                    break;
            }
            switch (iSampler.minFilter) {
                case gl::NearestMipmapNearest:
                case gl::LinearMipmapNearest:
                    mipmapFilter = nxt::FilterMode::Nearest;
                    break;
                case gl::NearestMipmapLinear:
                case gl::LinearMipmapLinear:
                    mipmapFilter = nxt::FilterMode::Linear;
                    break;
            }

            auto oSampler = device.CreateSamplerBuilder()
                .SetFilterMode(magFilter, minFilter, mipmapFilter)
                // TODO: wrap modes
                .GetResult();

            samplers[iSamplerID] = std::move(oSampler);
        }
    }

    void initTextures() {
        {
            auto oTexture = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(1, 1, 1)
                .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::TransferDst | nxt::TextureUsageBit::Sampled)
                .GetResult();
                // TODO: release this texture

            uint32_t white = 0xffffffff;
            nxt::Buffer staging = CreateFrozenBufferFromData(device, &white, sizeof(white), nxt::BufferUsageBit::TransferSrc);
            auto cmdbuf = device.CreateCommandBufferBuilder()
                .TransitionTextureUsage(oTexture, nxt::TextureUsageBit::TransferDst)
                .CopyBufferToTexture(staging, 0, oTexture, 0, 0, 0, 1, 1, 1, 0)
                .GetResult();
            queue.Submit(1, &cmdbuf);
            oTexture.FreezeUsage(nxt::TextureUsageBit::Sampled);

            defaultTexture = oTexture.CreateTextureViewBuilder().GetResult();
        }

        for (const auto& t : scene.textures) {
            const auto& iTextureID = t.first;
            const auto& iTexture = t.second;
            const auto& iImage = scene.images[iTexture.source];

            nxt::TextureFormat format = nxt::TextureFormat::R8G8B8A8Unorm;
            switch (iTexture.format) {
                case gl::RGBA:
                    format = nxt::TextureFormat::R8G8B8A8Unorm;
                    break;
                default:
                    fprintf(stderr, "unsupported texture format %d\n", iTexture.format);
                    continue;
            }

            auto oTexture = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(iImage.width, iImage.height, 1)
                .SetFormat(format)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::TransferDst | nxt::TextureUsageBit::Sampled)
                .GetResult();
                // TODO: release this texture

            uint32_t numPixels = iImage.width * iImage.height;
            const uint8_t* origData = iImage.image.data();
            const uint8_t* data = nullptr;
            std::vector<uint8_t> newData;
            if (iImage.component == 4) {
                data = origData;
            } else if (iImage.component == 3) {
                newData.resize(numPixels * 4);
                for (size_t i = 0; i < numPixels; ++i) {
                    newData[4 * i + 0] = origData[3 * i + 0];
                    newData[4 * i + 1] = origData[3 * i + 1];
                    newData[4 * i + 2] = origData[3 * i + 2];
                    newData[4 * i + 3] = 255;
                }
                data = newData.data();
            } else {
                fprintf(stderr, "unsupported image.component %d\n", iImage.component);
            }

            nxt::Buffer staging = CreateFrozenBufferFromData(device, data, numPixels * 4, nxt::BufferUsageBit::TransferSrc);
            auto cmdbuf = device.CreateCommandBufferBuilder()
                .TransitionTextureUsage(oTexture, nxt::TextureUsageBit::TransferDst)
                .CopyBufferToTexture(staging, 0, oTexture, 0, 0, 0, iImage.width, iImage.height, 1, 0)
                .GetResult();
            queue.Submit(1, &cmdbuf);
            oTexture.FreezeUsage(nxt::TextureUsageBit::Sampled);

            textures[iTextureID] = oTexture.CreateTextureViewBuilder().GetResult();
        }
    }

    void init() {
        device = CreateCppNXTDevice();

        queue = device.CreateQueueBuilder().GetResult();
        renderpass = device.CreateRenderPassBuilder()
            .SetAttachmentCount(1)
            .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
            .SetSubpassCount(1)
            .SubpassSetColorAttachment(0, 0, 0)
            .GetResult();
        framebuffer = device.CreateFramebufferBuilder()
            .SetRenderPass(renderpass)
            // attachment 0 -> back buffer
            // (implicit) // TODO(kainino@chromium.org): use the texture provided by WSI
            .SetDimensions(640, 480)
            .GetResult();

        initBuffers();
        initSamplers();
        initTextures();
    }
}

// Drawing
namespace {
    void drawMesh(const tinygltf::Mesh& iMesh, const glm::mat4& model) {
        nxt::CommandBufferBuilder cmd = device.CreateCommandBufferBuilder();
        for (const auto& iPrim : iMesh.primitives) {
            if (iPrim.mode != gl::Triangles) {
                fprintf(stderr, "unsupported primitive mode %d\n", iPrim.mode);
                continue;
            }

            u_transform_block transforms = {
                (projection * camera.view() * model),
                glm::inverseTranspose(model),
            };

            uint32_t strides[3] = {0};
            for (const auto& s : slotSemantics) {
                if (s.first < 3) {
                    auto it = iPrim.attributes.find(s.second);
                    if (it == iPrim.attributes.end()) {
                        continue;
                    }
                    const auto& iAccessorName = it->second;
                    strides[s.first] = scene.accessors.at(iAccessorName).byteStride;
                }
            }
            const MaterialInfo& material = getMaterial(iPrim.material, strides[0], strides[1], strides[2]);
            material.uniformBuffer.TransitionUsage(nxt::BufferUsageBit::TransferDst);
            material.uniformBuffer.SetSubData(0,
                    sizeof(u_transform_block) / sizeof(uint32_t),
                    reinterpret_cast<const uint32_t*>(&transforms));
            cmd.BeginRenderPass(renderpass, framebuffer);
            cmd.SetPipeline(material.pipeline);
            cmd.TransitionBufferUsage(material.uniformBuffer, nxt::BufferUsageBit::Uniform);
            cmd.SetBindGroup(0, material.bindGroup0);

            uint32_t vertexCount = 0;
            for (const auto& s : slotSemantics) {
                uint32_t slot = s.first;
                const auto& iSemantic = s.second;
                auto it = iPrim.attributes.find(s.second);
                if (it == iPrim.attributes.end()) {
                    uint32_t zero = 0;
                    cmd.SetVertexBuffers(slot, 1, &defaultBuffer, &zero);
                    continue;
                }
                const auto& iAccessor = scene.accessors.at(it->second);
                if (iAccessor.componentType != gl::Float ||
                        (iAccessor.type != TINYGLTF_TYPE_VEC4 && iAccessor.type != TINYGLTF_TYPE_VEC3 && iAccessor.type != TINYGLTF_TYPE_VEC2)) {
                    fprintf(stderr, "unsupported vertex accessor component type %d and type %d\n", iAccessor.componentType, iAccessor.type);
                    continue;
                }

                if (!vertexCount) {
                    vertexCount = iAccessor.count;
                }
                const auto& oBuffer = buffers.at(iAccessor.bufferView);
                uint32_t iBufferOffset = iAccessor.byteOffset;
                cmd.SetVertexBuffers(slot, 1, &oBuffer, &iBufferOffset);
            }

            if (!iPrim.indices.empty()) {
                const auto& iIndices = scene.accessors.at(iPrim.indices);
                // DrawElements
                if (iIndices.componentType != gl::UnsignedShort || iIndices.type != TINYGLTF_TYPE_SCALAR) {
                    fprintf(stderr, "unsupported index accessor component type %d and type %d\n", iIndices.componentType, iIndices.type);
                    continue;
                }
                const auto& oIndicesBuffer = buffers.at(iIndices.bufferView);
                cmd.SetIndexBuffer(oIndicesBuffer, iIndices.byteOffset, nxt::IndexFormat::Uint16);
                cmd.DrawElements(iIndices.count, 1, 0, 0);
            } else {
                // DrawArrays
                cmd.DrawArrays(vertexCount, 1, 0, 0);
            }
            cmd.EndRenderPass();
        }
        auto commands = cmd.GetResult();
        queue.Submit(1, &commands);
    }

    void drawNode(const tinygltf::Node& node, const glm::mat4& parent = glm::mat4()) {
        glm::mat4 model;
        if (node.matrix.size() == 16) {
            model = glm::make_mat4(node.matrix.data());
        } else {
            if (node.scale.size() == 3) {
                glm::vec3 scale = glm::make_vec3(node.scale.data());
                model = glm::scale(model, scale);
            }
            if (node.rotation.size() == 4) {
                glm::quat rotation = glm::make_quat(node.rotation.data());
                model = glm::mat4_cast(rotation) * model;
            }
            if (node.translation.size() == 3) {
                glm::vec3 translation = glm::make_vec3(node.translation.data());
                model = glm::translate(model, translation);
            }
        }
        model = parent * model;

        for (const auto& meshID : node.meshes) {
            drawMesh(scene.meshes[meshID], model);
        }
        for (const auto& child : node.children) {
            drawNode(scene.nodes.at(child), model);
        }
    }

    void frame() {
        const auto& defaultSceneNodes = scene.scenes.at(scene.defaultScene);
        for (const auto& n : defaultSceneNodes) {
            const auto& node = scene.nodes.at(n);
            drawNode(node);
        }
        DoSwapBuffers();
    }
}

// Mouse camera control
namespace {
    bool buttons[GLFW_MOUSE_BUTTON_LAST + 1] = {0};

    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        buttons[button] = (action == GLFW_PRESS);
    }

    void cursorPosCallback(GLFWwindow *window, double mouseX, double mouseY) {
        static float oldX, oldY;
        float dX = mouseX - oldX;
        float dY = mouseY - oldY;
        oldX = mouseX;
        oldY = mouseY;

        if (buttons[2] || (buttons[0] && buttons[1])) {
            camera.pan(-dX * 0.002, dY * 0.002);
        } else if (buttons[0]) {
            camera.rotate(dX * -0.01, dY * 0.01);
        } else if (buttons[1]) {
            camera.zoom(dY * -0.005);
        }
    }

    void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
        camera.zoom(yoffset * 0.04);
    }
}

int main(int argc, const char* argv[]) {
    if (!InitUtils(argc, argv)) {
        return 1;
    }
    if (argc < 2) {
        fprintf(stderr, "Usage: %s model.gltf [... NXT Options]\n", argv[0]);
        return 1;
    }

    tinygltf::TinyGLTFLoader loader;
    std::string err;
    std::string input_filename(argv[1]);
    std::string ext = getFilePathExtension(input_filename);

    bool ret = false;
    if (ext.compare("glb") == 0) {
        // assume binary glTF.
        ret = loader.LoadBinaryFromFile(&scene, &err, input_filename.c_str());
    } else {
        // assume ascii glTF.
        ret = loader.LoadASCIIFromFile(&scene, &err, input_filename.c_str());
    }
    if (!err.empty()) {
        fprintf(stderr, "ERR: %s\n", err.c_str());
    }
    if (!ret) {
        fprintf(stderr, "Failed to load .glTF : %s\n", argv[1]);
        exit(-1);
    }

    init();

    GLFWwindow* window = GetGLFWWindow();
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    while (!ShouldQuit()) {
        frame();
        USleep(16000);
    }

    // TODO release stuff
}
