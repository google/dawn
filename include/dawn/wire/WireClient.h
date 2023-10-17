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

#ifndef INCLUDE_DAWN_WIRE_WIRECLIENT_H_
#define INCLUDE_DAWN_WIRE_WIRECLIENT_H_

#include <memory>
#include <vector>

#include "dawn/dawn_proc_table.h"
#include "dawn/wire/Wire.h"

namespace dawn::wire {

namespace client {
class Client;
class MemoryTransferService;

DAWN_WIRE_EXPORT const DawnProcTable& GetProcs();
}  // namespace client

struct ReservedTexture {
    WGPUTexture texture;
    uint32_t id;
    uint32_t generation;
    uint32_t deviceId;
    uint32_t deviceGeneration;
};

struct ReservedSwapChain {
    WGPUSwapChain swapchain;
    uint32_t id;
    uint32_t generation;
    uint32_t deviceId;
    uint32_t deviceGeneration;
};

struct ReservedDevice {
    WGPUDevice device;
    uint32_t id;
    uint32_t generation;
};

struct ReservedInstance {
    WGPUInstance instance;
    uint32_t id;
    uint32_t generation;
};

struct DAWN_WIRE_EXPORT WireClientDescriptor {
    CommandSerializer* serializer;
    client::MemoryTransferService* memoryTransferService = nullptr;
};

class DAWN_WIRE_EXPORT WireClient : public CommandHandler {
  public:
    explicit WireClient(const WireClientDescriptor& descriptor);
    ~WireClient() override;

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override;

    ReservedTexture ReserveTexture(WGPUDevice device, const WGPUTextureDescriptor* descriptor);
    ReservedSwapChain ReserveSwapChain(WGPUDevice device,
                                       const WGPUSwapChainDescriptor* descriptor);
    ReservedDevice ReserveDevice();
    ReservedInstance ReserveInstance();

    void ReclaimTextureReservation(const ReservedTexture& reservation);
    void ReclaimSwapChainReservation(const ReservedSwapChain& reservation);
    void ReclaimDeviceReservation(const ReservedDevice& reservation);
    void ReclaimInstanceReservation(const ReservedInstance& reservation);

    // Disconnects the client.
    // Commands allocated after this point will not be sent.
    void Disconnect();

  private:
    std::unique_ptr<client::Client> mImpl;
};

namespace client {
class DAWN_WIRE_EXPORT MemoryTransferService {
  public:
    MemoryTransferService();
    virtual ~MemoryTransferService();

    class ReadHandle;
    class WriteHandle;

    // Create a handle for reading server data.
    // This may fail and return nullptr.
    virtual ReadHandle* CreateReadHandle(size_t) = 0;

    // Create a handle for writing server data.
    // This may fail and return nullptr.
    virtual WriteHandle* CreateWriteHandle(size_t) = 0;

    class DAWN_WIRE_EXPORT ReadHandle {
      public:
        ReadHandle();
        virtual ~ReadHandle();

        // Get the required serialization size for SerializeCreate
        virtual size_t SerializeCreateSize() = 0;

        // Serialize the handle into |serializePointer| so it can be received by the server.
        virtual void SerializeCreate(void* serializePointer) = 0;

        // Simply return the base address of the allocation (without applying any offset)
        // Returns nullptr if the allocation failed.
        // The data must live at least until the ReadHandle is destructued
        virtual const void* GetData() = 0;

        // Gets called when a MapReadCallback resolves.
        // deserialize the data update and apply
        // it to the range (offset, offset + size) of allocation
        // There could be nothing to be deserialized (if using shared memory)
        // Needs to check potential offset/size OOB and overflow
        virtual bool DeserializeDataUpdate(const void* deserializePointer,
                                           size_t deserializeSize,
                                           size_t offset,
                                           size_t size) = 0;

      private:
        ReadHandle(const ReadHandle&) = delete;
        ReadHandle& operator=(const ReadHandle&) = delete;
    };

    class DAWN_WIRE_EXPORT WriteHandle {
      public:
        WriteHandle();
        virtual ~WriteHandle();

        // Get the required serialization size for SerializeCreate
        virtual size_t SerializeCreateSize() = 0;

        // Serialize the handle into |serializePointer| so it can be received by the server.
        virtual void SerializeCreate(void* serializePointer) = 0;

        // Simply return the base address of the allocation (without applying any offset)
        // The data returned should be zero-initialized.
        // The data returned must live at least until the WriteHandle is destructed.
        // On failure, the pointer returned should be null.
        virtual void* GetData() = 0;

        // Get the required serialization size for SerializeDataUpdate
        virtual size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) = 0;

        // Serialize a command to send the modified contents of
        // the subrange (offset, offset + size) of the allocation at buffer unmap
        // This subrange is always the whole mapped region for now
        // There could be nothing to be serialized (if using shared memory)
        virtual void SerializeDataUpdate(void* serializePointer, size_t offset, size_t size) = 0;

      private:
        WriteHandle(const WriteHandle&) = delete;
        WriteHandle& operator=(const WriteHandle&) = delete;
    };

  private:
    MemoryTransferService(const MemoryTransferService&) = delete;
    MemoryTransferService& operator=(const MemoryTransferService&) = delete;
};

// Backdoor to get the order of the ProcMap for testing
DAWN_WIRE_EXPORT std::vector<const char*> GetProcMapNamesForTesting();
}  // namespace client
}  // namespace dawn::wire

#endif  // INCLUDE_DAWN_WIRE_WIRECLIENT_H_
