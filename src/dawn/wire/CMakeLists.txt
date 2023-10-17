# Copyright 2020 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
  INTERFACE
    "${DAWN_INCLUDE_DIR}/dawn/wire/Wire.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/WireClient.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/WireServer.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/dawn_wire_export.h"
  PRIVATE
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
    "client/EventManager.cpp"
    "client/EventManager.h"
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
    "client/SwapChain.cpp"
    "client/SwapChain.h"
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

install_if_enabled(dawn_wire)
