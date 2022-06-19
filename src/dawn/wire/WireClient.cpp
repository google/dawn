// Copyright 2019 The Dawn Authors
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

#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire {

WireClient::WireClient(const WireClientDescriptor& descriptor)
    : mImpl(new client::Client(descriptor.serializer, descriptor.memoryTransferService)) {}

WireClient::~WireClient() {
    mImpl.reset();
}

const volatile char* WireClient::HandleCommands(const volatile char* commands, size_t size) {
    return mImpl->HandleCommands(commands, size);
}

ReservedTexture WireClient::ReserveTexture(WGPUDevice device,
                                           const WGPUTextureDescriptor* descriptor) {
    return mImpl->ReserveTexture(device, descriptor);
}

ReservedSwapChain WireClient::ReserveSwapChain(WGPUDevice device) {
    return mImpl->ReserveSwapChain(device);
}

ReservedDevice WireClient::ReserveDevice() {
    return mImpl->ReserveDevice();
}

ReservedInstance WireClient::ReserveInstance() {
    return mImpl->ReserveInstance();
}

void WireClient::ReclaimTextureReservation(const ReservedTexture& reservation) {
    mImpl->ReclaimTextureReservation(reservation);
}

void WireClient::ReclaimSwapChainReservation(const ReservedSwapChain& reservation) {
    mImpl->ReclaimSwapChainReservation(reservation);
}

void WireClient::ReclaimDeviceReservation(const ReservedDevice& reservation) {
    mImpl->ReclaimDeviceReservation(reservation);
}

void WireClient::ReclaimInstanceReservation(const ReservedInstance& reservation) {
    mImpl->ReclaimInstanceReservation(reservation);
}

void WireClient::Disconnect() {
    mImpl->Disconnect();
}

namespace client {
MemoryTransferService::MemoryTransferService() = default;

MemoryTransferService::~MemoryTransferService() = default;

MemoryTransferService::ReadHandle::ReadHandle() = default;

MemoryTransferService::ReadHandle::~ReadHandle() = default;

MemoryTransferService::WriteHandle::WriteHandle() = default;

MemoryTransferService::WriteHandle::~WriteHandle() = default;
}  // namespace client

}  // namespace dawn::wire
