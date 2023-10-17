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

#ifndef SRC_DAWN_WIRE_SERVER_SERVER_H_
#define SRC_DAWN_WIRE_SERVER_SERVER_H_

#include <memory>
#include <utility>

#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/server/ServerBase_autogen.h"

namespace dawn::wire::server {

class Server;
class MemoryTransferService;

// CallbackUserdata and its derived classes are intended to be created by
// Server::MakeUserdata<T> and then passed as the userdata argument for Dawn
// callbacks.
// It contains a pointer back to the Server so that the callback can call the
// Server to perform operations like serialization, and it contains a weak pointer
// |serverIsAlive|. If the weak pointer has expired, it means the server has
// been destroyed and the callback must not use the Server pointer.
// To assist with checking |serverIsAlive| and lifetime management of the userdata,
// |ForwardToServer| (defined later in this file) can be used to acquire the userdata,
// return early if |serverIsAlive| has expired, and then forward the arguments
// to userdata->server->MyCallbackHandler.
//
// Example Usage:
//
// struct MyUserdata : CallbackUserdata { uint32_t foo; };
//
// auto userdata = MakeUserdata<MyUserdata>();
// userdata->foo = 2;
//
// callMyCallbackHandler(
//      ForwardToServer<&Server::MyCallbackHandler>,
//      userdata.release());
//
// void Server::MyCallbackHandler(MyUserdata* userdata, Other args) { }
struct CallbackUserdata {
    Server* const server;
    std::weak_ptr<bool> const serverIsAlive;

    CallbackUserdata() = delete;
    CallbackUserdata(Server* server, const std::shared_ptr<bool>& serverIsAlive);
};

template <auto F>
struct ForwardToServerHelper {
    template <typename _>
    struct ExtractedTypes;

    // An internal structure used to unpack the various types that compose the type of F
    template <typename Return, typename Class, typename Userdata, typename... Args>
    struct ExtractedTypes<Return (Class::*)(Userdata*, Args...)> {
        using UntypedCallback = Return (*)(Args..., void*);
        static Return Callback(Args... args, void* userdata) {
            // Acquire the userdata, and cast it to UserdataT.
            std::unique_ptr<Userdata> data(static_cast<Userdata*>(userdata));
            if (data->serverIsAlive.expired()) {
                // Do nothing if the server has already been destroyed.
                return;
            }
            // Forward the arguments and the typed userdata to the Server:: member function.
            (data->server->*F)(data.get(), std::forward<decltype(args)>(args)...);
        }
    };

    static constexpr typename ExtractedTypes<decltype(F)>::UntypedCallback Create() {
        return ExtractedTypes<decltype(F)>::Callback;
    }
};

template <auto F>
constexpr auto ForwardToServer = ForwardToServerHelper<F>::Create();

struct MapUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle buffer;
    WGPUBuffer bufferObj;
    WGPUFuture future;
    uint64_t offset;
    uint64_t size;
    WGPUMapModeFlags mode;
};

struct ErrorScopeUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle device;
    uint64_t requestSerial;
};

struct ShaderModuleGetCompilationInfoUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle shaderModule;
    uint64_t requestSerial;
};

struct QueueWorkDoneUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle queue;
    WGPUFuture future;
};

struct CreatePipelineAsyncUserData : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle device;
    uint64_t requestSerial;
    ObjectId pipelineObjectID;
};

struct RequestAdapterUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle instance;
    uint64_t requestSerial;
    ObjectId adapterObjectId;
};

struct RequestDeviceUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle adapter;
    uint64_t requestSerial;
    ObjectId deviceObjectId;
};

class Server : public ServerBase {
  public:
    Server(const DawnProcTable& procs,
           CommandSerializer* serializer,
           MemoryTransferService* memoryTransferService);
    ~Server() override;

    // ChunkedCommandHandler implementation
    const volatile char* HandleCommandsImpl(const volatile char* commands, size_t size) override;

    WireResult InjectTexture(WGPUTexture texture,
                             uint32_t id,
                             uint32_t generation,
                             uint32_t deviceId,
                             uint32_t deviceGeneration);

    WireResult InjectSwapChain(WGPUSwapChain swapchain,
                               uint32_t id,
                               uint32_t generation,
                               uint32_t deviceId,
                               uint32_t deviceGeneration);

    WireResult InjectDevice(WGPUDevice device, uint32_t id, uint32_t generation);

    WireResult InjectInstance(WGPUInstance instance, uint32_t id, uint32_t generation);

    WGPUDevice GetDevice(uint32_t id, uint32_t generation);
    bool IsDeviceKnown(WGPUDevice device) const;

    template <typename T,
              typename Enable = std::enable_if<std::is_base_of<CallbackUserdata, T>::value>>
    std::unique_ptr<T> MakeUserdata() {
        return std::unique_ptr<T>(new T(this, mIsAlive));
    }

  private:
    template <typename Cmd>
    void SerializeCommand(const Cmd& cmd) {
        mSerializer.SerializeCommand(cmd);
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(const Cmd& cmd, Extensions&&... es) {
        mSerializer.SerializeCommand(cmd, std::forward<Extensions>(es)...);
    }

    void SetForwardingDeviceCallbacks(Known<WGPUDevice> device);
    void ClearDeviceCallbacks(WGPUDevice device);

    // Error callbacks
    void OnUncapturedError(ObjectHandle device, WGPUErrorType type, const char* message);
    void OnDeviceLost(ObjectHandle device, WGPUDeviceLostReason reason, const char* message);
    void OnLogging(ObjectHandle device, WGPULoggingType type, const char* message);
    void OnDevicePopErrorScope(ErrorScopeUserdata* userdata,
                               WGPUErrorType type,
                               const char* message);
    void OnBufferMapAsyncCallback(MapUserdata* userdata, WGPUBufferMapAsyncStatus status);
    void OnQueueWorkDone(QueueWorkDoneUserdata* userdata, WGPUQueueWorkDoneStatus status);
    void OnCreateComputePipelineAsyncCallback(CreatePipelineAsyncUserData* userdata,
                                              WGPUCreatePipelineAsyncStatus status,
                                              WGPUComputePipeline pipeline,
                                              const char* message);
    void OnCreateRenderPipelineAsyncCallback(CreatePipelineAsyncUserData* userdata,
                                             WGPUCreatePipelineAsyncStatus status,
                                             WGPURenderPipeline pipeline,
                                             const char* message);
    void OnShaderModuleGetCompilationInfo(ShaderModuleGetCompilationInfoUserdata* userdata,
                                          WGPUCompilationInfoRequestStatus status,
                                          const WGPUCompilationInfo* info);
    void OnRequestAdapterCallback(RequestAdapterUserdata* userdata,
                                  WGPURequestAdapterStatus status,
                                  WGPUAdapter adapter,
                                  const char* message);
    void OnRequestDeviceCallback(RequestDeviceUserdata* userdata,
                                 WGPURequestDeviceStatus status,
                                 WGPUDevice device,
                                 const char* message);

#include "dawn/wire/server/ServerPrototypes_autogen.inc"

    WireDeserializeAllocator mAllocator;
    ChunkedCommandSerializer mSerializer;
    DawnProcTable mProcs;
    std::unique_ptr<MemoryTransferService> mOwnedMemoryTransferService = nullptr;
    MemoryTransferService* mMemoryTransferService = nullptr;

    std::shared_ptr<bool> mIsAlive;
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService();

}  // namespace dawn::wire::server

#endif  // SRC_DAWN_WIRE_SERVER_SERVER_H_
