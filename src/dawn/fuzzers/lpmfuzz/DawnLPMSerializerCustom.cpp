// Copyright 2023 The Dawn Authors
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

            // Hardcoded shader for now, eventually we should write an LPM grammar for WGSL
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
