// Copyright 2017 The Dawn Authors
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

#include "../SampleUtils.h"

#include "common/Assert.h"
#include "common/Math.h"
#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"
#include "utils/SystemUtils.h"

#include <bitset>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define PICOJSON_ASSERT ASSERT
#undef __STDC_FORMAT_MACROS
#include <tinygltfloader/tiny_gltf_loader.h>

#include "GLFW/glfw3.h"

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
    dawn::RenderPipeline pipeline;
    dawn::BindGroup bindGroup0;
    std::map<uint32_t, std::string> slotSemantics;
};

struct u_transform_block {
    glm::mat4 modelViewProj;
    glm::mat4 modelInvTr;
};

dawn::Device device;
dawn::Queue queue;
dawn::SwapChain swapchain;
dawn::TextureView depthStencilView;

dawn::Buffer defaultBuffer;
std::map<std::string, dawn::Buffer> buffers;
std::map<std::string, dawn::CommandBuffer> commandBuffers;
std::map<uint32_t, std::string> slotSemantics = {{0, "POSITION"}, {1, "NORMAL"}, {2, "TEXCOORD_0"}};

std::map<std::string, dawn::Sampler> samplers;
std::map<std::string, dawn::TextureView> textures;

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

    bool techniqueParameterTypeToVertexFormat(int type, dawn::VertexFormat *format) {
        switch (type) {
            case gl::FloatVec2:
                *format = dawn::VertexFormat::FloatR32G32;
                return true;
            case gl::FloatVec3:
                *format = dawn::VertexFormat::FloatR32G32B32;
                return true;
            case gl::FloatVec4:
                *format = dawn::VertexFormat::FloatR32G32B32A32;
                return true;
            default:
                return false;
        }
    }
}

// Initialization
namespace {
    void initBuffers() {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 256;
        descriptor.usage = dawn::BufferUsageBit::Vertex | dawn::BufferUsageBit::Index;
        defaultBuffer = device.CreateBuffer(&descriptor);

        for (const auto& bv : scene.bufferViews) {
            const auto& iBufferViewID = bv.first;
            const auto& iBufferView = bv.second;

            dawn::BufferUsageBit usage = dawn::BufferUsageBit::None;
            switch (iBufferView.target) {
                case gl::ArrayBuffer:
                    usage |= dawn::BufferUsageBit::Vertex;
                    break;
                case gl::ElementArrayBuffer:
                    usage |= dawn::BufferUsageBit::Index;
                    break;
                case 0:
                    fprintf(stderr, "TODO: buffer view has no target; skipping\n");
                    continue;
                default:
                    fprintf(stderr, "unsupported buffer view target %d\n", iBufferView.target);
                    continue;
            }
            const auto& iBuffer = scene.buffers.at(iBufferView.buffer);

            size_t iBufferViewSize =
                iBufferView.byteLength ? iBufferView.byteLength :
                (iBuffer.data.size() - iBufferView.byteOffset);
            auto oBuffer = utils::CreateBufferFromData(device, &iBuffer.data.at(iBufferView.byteOffset), static_cast<uint32_t>(iBufferViewSize), usage);
            buffers[iBufferViewID] = std::move(oBuffer);
        }
    }

    const MaterialInfo& getMaterial(const std::string& iMaterialID, size_t stridePos, size_t strideNor, size_t strideTxc) {
        static std::map<std::tuple<std::string, size_t, size_t, size_t>, MaterialInfo> materials;
        auto key = make_tuple(iMaterialID, stridePos, strideNor, strideTxc);
        auto materialIterator = materials.find(key);
        if (materialIterator != materials.end()) {
            return materialIterator->second;
        }

        const auto& iMaterial = scene.materials.at(iMaterialID);
        const auto& iTechnique = scene.techniques.at(iMaterial.technique);

        bool hasTexture = false;
        std::string iTextureID;
        {
            auto it = iMaterial.values.find("diffuse");
            if (it != iMaterial.values.end() && !it->second.string_value.empty()) {
                hasTexture = true;
                iTextureID = it->second.string_value;
            }
        }

        auto oVSModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450

            layout(push_constant) uniform u_transform_block {
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

        auto oFSSourceTextured = R"(
            #version 450

            layout(set = 0, binding = 0) uniform sampler u_samp;
            layout(set = 0, binding = 1) uniform texture2D u_tex;

            layout(location = 0) in vec3 v_normal;
            layout(location = 1) in vec2 v_texcoord;

            layout(location = 0) out vec4 fragcolor;

            void main() {
                const vec3 lightdir = normalize(vec3(-1, -2, 3));
                vec3 normal = normalize(v_normal);
                float diffuse = abs(dot(lightdir, normal));
                float diffamb = diffuse * 0.85 + 0.15;
                vec3 albedo = texture(sampler2D(u_tex, u_samp), v_texcoord).rgb;
                fragcolor = vec4(diffamb * albedo, 1);
            })";
        auto oFSSourceUntextured = R"(
            #version 450

            layout(location = 0) in vec3 v_normal;
            layout(location = 1) in vec2 v_texcoord;

            layout(location = 0) out vec4 fragcolor;

            void main() {
                const vec3 lightdir = normalize(vec3(-1, -2, 3));
                vec3 normal = normalize(v_normal);
                float diffuse = abs(dot(lightdir, normal));
                float diffamb = diffuse * 0.85 + 0.15;
                fragcolor = vec4(vec3(diffamb), 1);
            })";

        auto oFSModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, hasTexture ? oFSSourceTextured : oFSSourceUntextured);

        dawn::InputStateBuilder builder = device.CreateInputStateBuilder();
        std::bitset<3> slotsSet;
        for (const auto& a : iTechnique.attributes) {
            const auto iAttributeName = a.first;
            const auto iParameter = iTechnique.parameters.at(a.second);
            dawn::VertexFormat format;
            if (!techniqueParameterTypeToVertexFormat(iParameter.type, &format)) {
                fprintf(stderr, "unsupported technique parameter type %d\n", iParameter.type);
                continue;
            }
            if (iParameter.semantic == "POSITION") {
                builder.SetAttribute(0, 0, format, 0);
                builder.SetInput(0, static_cast<uint32_t>(stridePos), dawn::InputStepMode::Vertex);
                slotsSet.set(0);
            } else if (iParameter.semantic == "NORMAL") {
                builder.SetAttribute(1, 1, format, 0);
                builder.SetInput(1, static_cast<uint32_t>(strideNor), dawn::InputStepMode::Vertex);
                slotsSet.set(1);
            } else if (iParameter.semantic == "TEXCOORD_0") {
                builder.SetAttribute(2, 2, format, 0);
                builder.SetInput(2, static_cast<uint32_t>(strideTxc), dawn::InputStepMode::Vertex);
                slotsSet.set(2);
            } else {
                fprintf(stderr, "unsupported technique attribute semantic %s\n", iParameter.semantic.c_str());
            }
            // TODO: use iAttributeParameter.node?
        }
        for (uint32_t i = 0; i < slotsSet.size(); i++) {
            if (slotsSet[i]) {
                continue;
            }
            builder.SetAttribute(i, i, dawn::VertexFormat::FloatR32G32B32A32, 0);
            builder.SetInput(i, 0, dawn::InputStepMode::Vertex);
        }
        auto inputState = builder.GetResult();

        constexpr dawn::ShaderStageBit kNoStages{};
        dawn::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {
                        {0, hasTexture ? dawn::ShaderStageBit::Fragment : kNoStages,
                         dawn::BindingType::Sampler},
                        {1, hasTexture ? dawn::ShaderStageBit::Fragment : kNoStages,
                         dawn::BindingType::SampledTexture},
                    });

        auto depthStencilState = device.CreateDepthStencilStateBuilder()
            .SetDepthCompareFunction(dawn::CompareFunction::Less)
            .SetDepthWriteEnabled(true)
            .GetResult();

        auto pipelineLayout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.layout = pipelineLayout;
        descriptor.cVertexStage.module = oVSModule;
        descriptor.cFragmentStage.module = oFSModule;
        descriptor.inputState = inputState;
        descriptor.indexFormat = dawn::IndexFormat::Uint16;
        descriptor.cAttachmentsState.hasDepthStencilAttachment = true;
        descriptor.cDepthStencilAttachment.format = dawn::TextureFormat::D32FloatS8Uint;
        descriptor.cColorAttachments[0].format =
            GetPreferredSwapChainTextureFormat();
        descriptor.depthStencilState = depthStencilState;

        dawn::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        dawn::BindGroup bindGroup;

        if (hasTexture) {
            const auto& textureView = textures[iTextureID];
            const auto& iSamplerID = scene.textures[iTextureID].sampler;
            bindGroup = utils::MakeBindGroup(device, bindGroupLayout, {
                {0, samplers[iSamplerID]},
                {1, textureView}
            });
        } else {
            bindGroup = utils::MakeBindGroup(device, bindGroupLayout, {});
        }

        MaterialInfo material = {
            pipeline,
            bindGroup,
            std::map<uint32_t, std::string>(),
        };
        materials[key] = std::move(material);
        return materials.at(key);
    }

    void initSamplers() {
        for (const auto& s : scene.samplers) {
            const auto& iSamplerID = s.first;
            const auto& iSampler = s.second;

            dawn::SamplerDescriptor desc = utils::GetDefaultSamplerDescriptor();
            // TODO: wrap modes

            switch (iSampler.magFilter) {
                case gl::Nearest:
                    desc.magFilter = dawn::FilterMode::Nearest;
                    break;
                case gl::Linear:
                    desc.magFilter = dawn::FilterMode::Linear;
                    break;
                default:
                    fprintf(stderr, "unsupported magFilter %d\n", iSampler.magFilter);
                    break;
            }
            switch (iSampler.minFilter) {
                case gl::Nearest:
                case gl::NearestMipmapNearest:
                case gl::NearestMipmapLinear:
                    desc.minFilter = dawn::FilterMode::Nearest;
                    break;
                case gl::Linear:
                case gl::LinearMipmapNearest:
                case gl::LinearMipmapLinear:
                    desc.minFilter = dawn::FilterMode::Linear;
                    break;
                default:
                    fprintf(stderr, "unsupported minFilter %d\n", iSampler.magFilter);
                    break;
            }
            switch (iSampler.minFilter) {
                case gl::NearestMipmapNearest:
                case gl::LinearMipmapNearest:
                    desc.mipmapFilter = dawn::FilterMode::Nearest;
                    break;
                case gl::NearestMipmapLinear:
                case gl::LinearMipmapLinear:
                    desc.mipmapFilter = dawn::FilterMode::Linear;
                    break;
            }

            samplers[iSamplerID] = device.CreateSampler(&desc);
        }
    }

    void initTextures() {
        for (const auto& t : scene.textures) {
            const auto& iTextureID = t.first;
            const auto& iTexture = t.second;
            const auto& iImage = scene.images[iTexture.source];

            dawn::TextureFormat format = dawn::TextureFormat::R8G8B8A8Unorm;
            switch (iTexture.format) {
                case gl::RGBA:
                    format = dawn::TextureFormat::R8G8B8A8Unorm;
                    break;
                default:
                    fprintf(stderr, "unsupported texture format %d\n", iTexture.format);
                    continue;
            }

            dawn::TextureDescriptor descriptor;
            descriptor.dimension = dawn::TextureDimension::e2D;
            descriptor.size.width = iImage.width;
            descriptor.size.height = iImage.height;
            descriptor.size.depth = 1;
            descriptor.arrayLayer = 1;
            descriptor.format = format;
            descriptor.levelCount = 1;
            descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Sampled;
            auto oTexture = device.CreateTexture(&descriptor);
                // TODO: release this texture

            const uint8_t* origData = iImage.image.data();
            const uint8_t* data = nullptr;
            std::vector<uint8_t> newData;

            uint32_t width = static_cast<uint32_t>(iImage.width);
            uint32_t height = static_cast<uint32_t>(iImage.height);
            uint32_t rowSize = width * 4;
            uint32_t rowPitch = Align(rowSize, kTextureRowPitchAlignment);

            if (iImage.component == 3 || iImage.component == 4) {
                if (rowSize != rowPitch || iImage.component == 3) {
                    newData.resize(rowPitch * height);
                    uint32_t pixelsPerRow = rowPitch / 4;
                    for (uint32_t y = 0; y < height; ++y) {
                        for (uint32_t x = 0; x < width; ++x) {
                            size_t oldIndex = x + y * height;
                            size_t newIndex = x + y * pixelsPerRow;
                            if (iImage.component == 4) {
                                newData[4 * newIndex + 0] = origData[4 * oldIndex + 0];
                                newData[4 * newIndex + 1] = origData[4 * oldIndex + 1];
                                newData[4 * newIndex + 2] = origData[4 * oldIndex + 2];
                                newData[4 * newIndex + 3] = origData[4 * oldIndex + 3];
                            } else if (iImage.component == 3) {
                                newData[4 * newIndex + 0] = origData[3 * oldIndex + 0];
                                newData[4 * newIndex + 1] = origData[3 * oldIndex + 1];
                                newData[4 * newIndex + 2] = origData[3 * oldIndex + 2];
                                newData[4 * newIndex + 3] = 255;
                            }
                        }
                    }
                    data = newData.data();
                } else {
                    data = origData;
                }
            } else {
                fprintf(stderr, "unsupported image.component %d\n", iImage.component);
            }

            dawn::Buffer staging = utils::CreateBufferFromData(device, data, rowPitch * iImage.height, dawn::BufferUsageBit::TransferSrc);
            dawn::BufferCopyView bufferCopyView =
                utils::CreateBufferCopyView(staging, 0, rowPitch, 0);
            dawn::TextureCopyView textureCopyView =
                utils::CreateTextureCopyView(oTexture, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color);
            dawn::Extent3D copySize = {iImage.width, iImage.height, 1};
            auto cmdbuf = device.CreateCommandBufferBuilder()
                              .CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize)
                              .GetResult();
            queue.Submit(1, &cmdbuf);

            textures[iTextureID] = oTexture.CreateDefaultTextureView();
        }
    }

    void init() {
        device = CreateCppDawnDevice();

        queue = device.CreateQueue();
        swapchain = GetSwapChain(device);
        swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                            dawn::TextureUsageBit::OutputAttachment, 640, 480);

        depthStencilView = CreateDefaultDepthStencilView(device);

        initBuffers();
        initSamplers();
        initTextures();
    }
}

// Drawing
namespace {
    void drawMesh(dawn::RenderPassEncoder& pass, const tinygltf::Mesh& iMesh, const glm::mat4& model) {
        for (const auto& iPrim : iMesh.primitives) {
            if (iPrim.mode != gl::Triangles) {
                fprintf(stderr, "unsupported primitive mode %d\n", iPrim.mode);
                continue;
            }

            u_transform_block transforms = {
                (projection * camera.view() * model),
                glm::inverseTranspose(model),
            };

            size_t strides[3] = {0};
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
            pass.SetRenderPipeline(material.pipeline);
            pass.SetBindGroup(0, material.bindGroup0);
            pass.SetPushConstants(dawn::ShaderStageBit::Vertex,
                    0, sizeof(u_transform_block) / sizeof(uint32_t),
                    reinterpret_cast<const uint32_t*>(&transforms));

            uint32_t vertexCount = 0;
            for (const auto& s : slotSemantics) {
                uint32_t slot = s.first;
                auto it = iPrim.attributes.find(s.second);
                if (it == iPrim.attributes.end()) {
                    uint32_t zero = 0;
                    pass.SetVertexBuffers(slot, 1, &defaultBuffer, &zero);
                    continue;
                }
                const auto& iAccessor = scene.accessors.at(it->second);
                if (iAccessor.componentType != gl::Float ||
                        (iAccessor.type != TINYGLTF_TYPE_VEC4 && iAccessor.type != TINYGLTF_TYPE_VEC3 && iAccessor.type != TINYGLTF_TYPE_VEC2)) {
                    fprintf(stderr, "unsupported vertex accessor component type %d and type %d\n", iAccessor.componentType, iAccessor.type);
                    continue;
                }

                if (vertexCount == 0) {
                    vertexCount = static_cast<uint32_t>(iAccessor.count);
                }
                const auto& oBuffer = buffers.at(iAccessor.bufferView);
                uint32_t iBufferOffset = static_cast<uint32_t>(iAccessor.byteOffset);
                pass.SetVertexBuffers(slot, 1, &oBuffer, &iBufferOffset);
            }

            if (!iPrim.indices.empty()) {
                const auto& iIndices = scene.accessors.at(iPrim.indices);
                // DrawElements
                if (iIndices.componentType != gl::UnsignedShort || iIndices.type != TINYGLTF_TYPE_SCALAR) {
                    fprintf(stderr, "unsupported index accessor component type %d and type %d\n", iIndices.componentType, iIndices.type);
                    continue;
                }
                const auto& oIndicesBuffer = buffers.at(iIndices.bufferView);
                pass.SetIndexBuffer(oIndicesBuffer, static_cast<uint32_t>(iIndices.byteOffset));
                pass.DrawIndexed(static_cast<uint32_t>(iIndices.count), 1, 0, 0);
            } else {
                // DrawArrays
                pass.Draw(vertexCount, 1, 0, 0);
            }
        }
    }

    void drawNode(dawn::RenderPassEncoder& pass, const tinygltf::Node& node, const glm::mat4& parent = glm::mat4()) {
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
            drawMesh(pass, scene.meshes[meshID], model);
        }
        for (const auto& child : node.children) {
            drawNode(pass, scene.nodes.at(child), model);
        }
    }

    void frame() {
        dawn::Texture backbuffer;
        dawn::RenderPassDescriptor renderPass;
        GetNextRenderPassDescriptor(device, swapchain, depthStencilView, &backbuffer, &renderPass);

        const auto& defaultSceneNodes = scene.scenes.at(scene.defaultScene);
        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        {
            dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass);
            for (const auto& n : defaultSceneNodes) {
                const auto& node = scene.nodes.at(n);
                drawNode(pass, node);
            }
            pass.EndPass();
        }

        dawn::CommandBuffer commands = builder.GetResult();
        queue.Submit(1, &commands);

        swapchain.Present(backbuffer);
        DoFlush();
    }
}

// Mouse camera control
namespace {
    bool buttons[GLFW_MOUSE_BUTTON_LAST + 1] = {0};

    void mouseButtonCallback(GLFWwindow*, int button, int action, int) {
        buttons[button] = (action == GLFW_PRESS);
    }

    void cursorPosCallback(GLFWwindow*, double mouseX, double mouseY) {
        static double oldX, oldY;
        float dX = static_cast<float>(mouseX - oldX);
        float dY = static_cast<float>(mouseY - oldY);
        oldX = mouseX;
        oldY = mouseY;

        if (buttons[2] || (buttons[0] && buttons[1])) {
            camera.pan(-dX * 0.002f, dY * 0.002f);
        } else if (buttons[0]) {
            camera.rotate(dX * 0.01f, dY * 0.01f);
        } else if (buttons[1]) {
            camera.zoom(dY * -0.005f);
        }
    }

    void scrollCallback(GLFWwindow*, double, double yoffset) {
        camera.zoom(static_cast<float>(yoffset) * 0.04f);
    }
}

int main(int argc, const char* argv[]) {
    if (!InitSample(argc, argv)) {
        return 1;
    }
    if (argc < 2) {
        fprintf(stderr, "Usage: %s model.gltf [... Dawn Options]\n", argv[0]);
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
        utils::USleep(16000);
    }

    // TODO release stuff
}
