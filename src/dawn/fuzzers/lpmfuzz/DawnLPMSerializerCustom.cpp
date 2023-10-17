// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>

#include "dawn/fuzzers/lpmfuzz/DawnLPMConstants_autogen.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMObjectStore.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializerCustom.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h"
#include "dawn/webgpu.h"
#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/ObjectType_autogen.h"

namespace dawn::wire {

void GetCustomSerializedData(const fuzzing::Command& command,
                             dawn::wire::ChunkedCommandSerializer serializer,
                             PerObjectType<DawnLPMObjectStore>& objectStores,
                             DawnLPMObjectIdProvider& provider) {
    switch (command.command_case()) {
        case fuzzing::Command::kDeviceCreateShaderModule: {
            DeviceCreateShaderModuleCmd cmd;
            memset(&cmd, 0, sizeof(DeviceCreateShaderModuleCmd));

            ObjectId cmd_self_id =
                objectStores[ObjectType::Device].Lookup(command.devicecreateshadermodule().self());

            if (cmd_self_id == static_cast<ObjectId>(DawnLPMFuzzer::kInvalidObjectId)) {
                break;
            }

            cmd.self = reinterpret_cast<WGPUDevice>(cmd_self_id);

            WGPUShaderModuleDescriptor cmd_descriptor;
            memset(&cmd_descriptor, 0, sizeof(struct WGPUShaderModuleDescriptor));

            WGPUShaderModuleWGSLDescriptor wgsl_desc = {};
            wgsl_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;

            if (DAWNLPM_FUZZ_TINT && command.devicecreateshadermodule().has_code()) {
                wgsl_desc.code = command.devicecreateshadermodule().code().c_str();
            } else {
                wgsl_desc.code =
                    "@group(0) @binding(0)\n"
                    "var<storage, read_write> output: array<f32>;\n"
                    "@compute @workgroup_size(64)\n"
                    "fn main( \n"
                    "    @builtin(global_invocation_id) global_id : vec3<u32>,\n"
                    "    @builtin(local_invocation_id) local_id : vec3<u32>,\n"
                    ") { \n"
                    "output[global_id.x] = \n"
                    "    f32(global_id.x) * 1000. + f32(local_id.x);\n"
                    "}";
            }
            cmd_descriptor.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgsl_desc);

            cmd.descriptor = &cmd_descriptor;
            if (objectStores[ObjectType::ShaderModule].Size() >=
                DawnLPMFuzzer::kShaderModuleLimit) {
                break;
            }

            cmd.result = objectStores[ObjectType::ShaderModule].ReserveHandle();
            serializer.SerializeCommand(cmd, provider);
            break;
        }
        case fuzzing::Command::kDestroyObject: {
            DestroyObjectCmd cmd;
            memset(&cmd, 0, sizeof(DestroyObjectCmd));

            cmd.objectType =
                static_cast<ObjectType>(command.destroyobject().objecttype() % kObjectTypes);

            cmd.objectId = objectStores[static_cast<ObjectType>(cmd.objectType)].Lookup(
                command.destroyobject().objectid());

            if (cmd.objectId == static_cast<ObjectId>(DawnLPMFuzzer::kInvalidObjectId)) {
                break;
            }

            objectStores[cmd.objectType].Free(cmd.objectId);
            serializer.SerializeCommand(cmd, provider);
            break;
        }
        default: {
            break;
        }
    }
}

}  // namespace dawn::wire
