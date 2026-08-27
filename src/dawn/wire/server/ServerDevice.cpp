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

#include <utility>

#include "dawn/wire/Wire.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/WireResult.h"
#include "src/dawn/wire/server/Server.h"

namespace dawn::wire::server {

void Server::OnUncapturedError(ObjectHandle device, WGPUErrorType type, WGPUStringView message) {
    ReturnDeviceUncapturedErrorCallbackCmd cmd;
    cmd.device = device;
    cmd.type = FromAPI(type);
    cmd.message = FromAPI(message);

    SerializeCommand(std::move(cmd));
    Flush();
}

void Server::OnDeviceLost(DeviceLostUserdata* userdata,
                          WGPUDevice const* device,
                          WGPUDeviceLostReason reason,
                          WGPUStringView message) {
    ReturnDeviceLostCallbackCmd cmd;
    cmd.instanceId = userdata->instanceId;
    cmd.future = FromAPI(userdata->future);
    cmd.reason = FromAPI(reason);
    cmd.message = FromAPI(message);

    SerializeCommand(std::move(cmd));
}

void Server::OnLogging(ObjectHandle device, WGPULoggingType type, WGPUStringView message) {
    ReturnDeviceLoggingCallbackCmd cmd;
    cmd.device = device;
    cmd.type = FromAPI(type);
    cmd.message = FromAPI(message);

    SerializeCommand(std::move(cmd));
}

WireResult Server::DoDevicePopErrorScope(Known<WGPUDevice> device,
                                         Known<WGPUInstance> instance,
                                         Future future) {
    auto userdata = MakeUserdata<ErrorScopeUserdata>();
    userdata->device = device.AsHandle();
    userdata->instanceId = instance.id;
    userdata->future = ToAPI(future);

    mProcs->devicePopErrorScope(
        device->handle,
        MakeCallbackInfo<WGPUPopErrorScopeCallbackInfo, &Server::OnDevicePopErrorScope>(
            userdata.release()));
    return WireResult::Success;
}

void Server::OnDevicePopErrorScope(ErrorScopeUserdata* userdata,
                                   WGPUPopErrorScopeStatus status,
                                   WGPUErrorType type,
                                   WGPUStringView message) {
    ReturnDevicePopErrorScopeCallbackCmd cmd;
    cmd.instanceId = userdata->instanceId;
    cmd.future = FromAPI(userdata->future);
    cmd.status = FromAPI(status);
    cmd.type = FromAPI(type);
    cmd.message = FromAPI(message);

    SerializeCommand(std::move(cmd));
}

WireResult Server::DoDeviceCreateComputePipelineAsync(Known<WGPUDevice> device,
                                                      Known<WGPUInstance> instance,
                                                      Future future,
                                                      ObjectHandle pipelineObjectHandle,
                                                      const ComputePipelineDescriptor* descriptor) {
    Reserved<WGPUComputePipeline> pipeline;
    WIRE_TRY(Allocate(&pipeline, pipelineObjectHandle, AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->instanceId = instance.id;
    userdata->future = ToAPI(future);
    userdata->pipeline = pipeline.AsHandle();

    mProcs->deviceCreateComputePipelineAsync(
        device->handle, ToAPI(descriptor),
        MakeCallbackInfo<WGPUCreateComputePipelineAsyncCallbackInfo,
                         &Server::OnCreateComputePipelineAsyncCallback>(userdata.release()));
    return WireResult::Success;
}

void Server::OnCreateComputePipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                  WGPUCreatePipelineAsyncStatus status,
                                                  WGPUComputePipeline pipeline,
                                                  WGPUStringView message) {
    ReturnDeviceCreateComputePipelineAsyncCallbackCmd cmd;
    cmd.instanceId = data->instanceId;
    cmd.future = FromAPI(data->future);
    cmd.status = FromAPI(status);
    cmd.message = FromAPI(message);

    if (status == WGPUCreatePipelineAsyncStatus_Success &&
        FillReservation(data->pipeline, pipeline) == WireResult::FatalError) {
        cmd.status = wgpu::CreatePipelineAsyncStatus::CallbackCancelled;
        cmd.message = FromAPI(ToOutputStringView("Destroyed before request was fulfilled."));
    }
    SerializeCommand(std::move(cmd));
}

WireResult Server::DoDeviceCreateRenderPipelineAsync(Known<WGPUDevice> device,
                                                     Known<WGPUInstance> instance,
                                                     Future future,
                                                     ObjectHandle pipelineObjectHandle,
                                                     const RenderPipelineDescriptor* descriptor) {
    Reserved<WGPURenderPipeline> pipeline;
    WIRE_TRY(Allocate(&pipeline, pipelineObjectHandle, AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->instanceId = instance.id;
    userdata->future = ToAPI(future);
    userdata->pipeline = pipeline.AsHandle();

    mProcs->deviceCreateRenderPipelineAsync(
        device->handle, ToAPI(descriptor),
        MakeCallbackInfo<WGPUCreateRenderPipelineAsyncCallbackInfo,
                         &Server::OnCreateRenderPipelineAsyncCallback>(userdata.release()));
    return WireResult::Success;
}

void Server::OnCreateRenderPipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                 WGPUCreatePipelineAsyncStatus status,
                                                 WGPURenderPipeline pipeline,
                                                 WGPUStringView message) {
    ReturnDeviceCreateRenderPipelineAsyncCallbackCmd cmd;
    cmd.instanceId = data->instanceId;
    cmd.future = FromAPI(data->future);
    cmd.status = FromAPI(status);
    cmd.message = FromAPI(message);

    if (status == WGPUCreatePipelineAsyncStatus_Success &&
        FillReservation(data->pipeline, pipeline) == WireResult::FatalError) {
        cmd.status = wgpu::CreatePipelineAsyncStatus::CallbackCancelled;
        cmd.message = FromAPI(ToOutputStringView("Destroyed before request was fulfilled."));
    }
    SerializeCommand(std::move(cmd));
}

}  // namespace dawn::wire::server
