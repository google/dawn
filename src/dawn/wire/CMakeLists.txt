# Copyright 2020 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DawnJSONGenerator(
    TARGET "wire"
    PRINT_NAME "Dawn wire"
    RESULT_VARIABLE "DAWN_WIRE_GEN_SOURCES"
)

add_library(dawn_wire ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_wire)

target_compile_definitions(dawn_wire PRIVATE "DAWN_WIRE_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_wire PRIVATE "DAWN_WIRE_SHARED_LIBRARY")
endif()

target_sources(dawn_wire PRIVATE
    "${DAWN_INCLUDE_DIR}/dawn/wire/Wire.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/WireClient.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/WireServer.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/dawn_wire_export.h"
    ${DAWN_WIRE_GEN_SOURCES}
    "BufferConsumer.h"
    "BufferConsumer_impl.h"
    "ChunkedCommandHandler.cpp"
    "ChunkedCommandHandler.h"
    "ChunkedCommandSerializer.cpp"
    "ChunkedCommandSerializer.h"
    "ObjectHandle.cpp"
    "ObjectHandle.h"
    "SupportedFeatures.cpp"
    "SupportedFeatures.h"
    "Wire.cpp"
    "WireClient.cpp"
    "WireDeserializeAllocator.cpp"
    "WireDeserializeAllocator.h"
    "WireResult.h"
    "WireServer.cpp"
    "client/Adapter.cpp"
    "client/Adapter.h"
    "client/ApiObjects.h"
    "client/Buffer.cpp"
    "client/Buffer.h"
    "client/Client.cpp"
    "client/Client.h"
    "client/ClientDoers.cpp"
    "client/ClientInlineMemoryTransferService.cpp"
    "client/Device.cpp"
    "client/Device.h"
    "client/Instance.cpp"
    "client/Instance.h"
    "client/LimitsAndFeatures.cpp"
    "client/LimitsAndFeatures.h"
    "client/ObjectStore.cpp"
    "client/ObjectStore.h"
    "client/ObjectBase.cpp"
    "client/ObjectBase.h"
    "client/QuerySet.cpp"
    "client/QuerySet.h"
    "client/Queue.cpp"
    "client/Queue.h"
    "client/RequestTracker.h"
    "client/ShaderModule.cpp"
    "client/ShaderModule.h"
    "client/Texture.cpp"
    "client/Texture.h"
    "server/ObjectStorage.h"
    "server/Server.cpp"
    "server/Server.h"
    "server/ServerAdapter.cpp"
    "server/ServerBuffer.cpp"
    "server/ServerDevice.cpp"
    "server/ServerInlineMemoryTransferService.cpp"
    "server/ServerInstance.cpp"
    "server/ServerQueue.cpp"
    "server/ServerShaderModule.cpp"
)
target_link_libraries(dawn_wire
    PUBLIC dawn_headers
    PRIVATE dawn_common dawn_internal_config
)
