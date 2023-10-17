// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

namespace {

template <ObjectType objectType, typename Pipeline>
void HandleCreatePipelineAsyncCallback(KnownObjects<Pipeline>* knownObjects,
                                       WGPUCreatePipelineAsyncStatus status,
                                       Pipeline pipeline,
                                       CreatePipelineAsyncUserData* data) {
    if (status == WGPUCreatePipelineAsyncStatus_Success) {
        knownObjects->FillReservation(data->pipelineObjectID, pipeline);
    } else {
        // Otherwise, free the ObjectId which will make it unusable.
        knownObjects->Free(data->pipelineObjectID);
        DAWN_ASSERT(pipeline == nullptr);
    }
}

}  // anonymous namespace

void Server::OnUncapturedError(ObjectHandle device, WGPUErrorType type, const char* message) {
    ReturnDeviceUncapturedErrorCallbackCmd cmd;
    cmd.device = device;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

void Server::OnDeviceLost(ObjectHandle device, WGPUDeviceLostReason reason, const char* message) {
    ReturnDeviceLostCallbackCmd cmd;
    cmd.device = device;
    cmd.reason = reason;
    cmd.message = message;

    SerializeCommand(cmd);
}

void Server::OnLogging(ObjectHandle device, WGPULoggingType type, const char* message) {
    ReturnDeviceLoggingCallbackCmd cmd;
    cmd.device = device;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDevicePopErrorScope(Known<WGPUDevice> device, uint64_t requestSerial) {
    auto userdata = MakeUserdata<ErrorScopeUserdata>();
    userdata->requestSerial = requestSerial;
    userdata->device = device.AsHandle();

    mProcs.devicePopErrorScope(device->handle, ForwardToServer<&Server::OnDevicePopErrorScope>,
                               userdata.release());
    return WireResult::Success;
}

void Server::OnDevicePopErrorScope(ErrorScopeUserdata* userdata,
                                   WGPUErrorType type,
                                   const char* message) {
    ReturnDevicePopErrorScopeCallbackCmd cmd;
    cmd.device = userdata->device;
    cmd.requestSerial = userdata->requestSerial;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDeviceCreateComputePipelineAsync(
    Known<WGPUDevice> device,
    uint64_t requestSerial,
    ObjectHandle pipelineObjectHandle,
    const WGPUComputePipelineDescriptor* descriptor) {
    Known<WGPUComputePipeline> pipeline;
    WIRE_TRY(ComputePipelineObjects().Allocate(&pipeline, pipelineObjectHandle,
                                               AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->requestSerial = requestSerial;
    userdata->pipelineObjectID = pipeline.id;

    mProcs.deviceCreateComputePipelineAsync(
        device->handle, descriptor, ForwardToServer<&Server::OnCreateComputePipelineAsyncCallback>,
        userdata.release());
    return WireResult::Success;
}

void Server::OnCreateComputePipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                  WGPUCreatePipelineAsyncStatus status,
                                                  WGPUComputePipeline pipeline,
                                                  const char* message) {
    HandleCreatePipelineAsyncCallback<ObjectType::ComputePipeline>(&ComputePipelineObjects(),
                                                                   status, pipeline, data);

    ReturnDeviceCreateComputePipelineAsyncCallbackCmd cmd;
    cmd.device = data->device;
    cmd.status = status;
    cmd.requestSerial = data->requestSerial;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDeviceCreateRenderPipelineAsync(
    Known<WGPUDevice> device,
    uint64_t requestSerial,
    ObjectHandle pipelineObjectHandle,
    const WGPURenderPipelineDescriptor* descriptor) {
    Known<WGPURenderPipeline> pipeline;
    WIRE_TRY(RenderPipelineObjects().Allocate(&pipeline, pipelineObjectHandle,
                                              AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->requestSerial = requestSerial;
    userdata->pipelineObjectID = pipeline.id;

    mProcs.deviceCreateRenderPipelineAsync(
        device->handle, descriptor, ForwardToServer<&Server::OnCreateRenderPipelineAsyncCallback>,
        userdata.release());
    return WireResult::Success;
}

void Server::OnCreateRenderPipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                 WGPUCreatePipelineAsyncStatus status,
                                                 WGPURenderPipeline pipeline,
                                                 const char* message) {
    HandleCreatePipelineAsyncCallback<ObjectType::RenderPipeline>(&RenderPipelineObjects(), status,
                                                                  pipeline, data);

    ReturnDeviceCreateRenderPipelineAsyncCallbackCmd cmd;
    cmd.device = data->device;
    cmd.status = status;
    cmd.requestSerial = data->requestSerial;
    cmd.message = message;

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
