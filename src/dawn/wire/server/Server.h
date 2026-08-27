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

#include "dawn/wire/server/ServerBase_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/wire/ChunkedCommandSerializer.h"

namespace dawn::wire {
namespace server {
class Server;
class MemoryTransferService;
}  // namespace server

namespace detail {

template <typename Target, typename Arg>
Target ConvertFromAPI(Arg&& arg) {
    if constexpr (std::is_same_v<Target, std::decay_t<Arg>>) {
        return std::forward<Arg>(arg);
    } else {
        return FromAPI(std::forward<Arg>(arg));
    }
}

template <auto F, typename _ = decltype(F)>
struct ForwardToServerHelper;

template <auto F, typename UserdataT, typename... Args>
struct ForwardToServerHelper<F, void (server::Server::*)(UserdataT*, Args...)> {
    using Userdata = UserdataT;

    static void Callback(AsAPIType<Args>... args, void* userdata, void*) {
        // Acquire the userdata, and cast it to UserdataT.
        std::unique_ptr<Userdata> data(static_cast<Userdata*>(userdata));
        auto server = data->server.lock();
        if (!server) {
            // If the server is destroyed, release any callback owned results and return.
            (
                []<typename T>(const DawnProcTable& procs, T arg) {
                    if constexpr (server::WGPUTraits<T>::Release != nullptr) {
                        if (arg) {
                            (procs.*server::WGPUTraits<T>::Release)(arg);
                        }
                    }
                }(*(data->procs), args),
                ...);
            return;
        }
        // Forward the arguments and the typed userdata to the Server:: member function.
        {
            auto serverGuard = server.get()->GetGuard();
            (server.get()->*F)(data.get(), ConvertFromAPI<Args>(args)...);
        }
        server.get()->Flush();
    }
};

}  // namespace detail

namespace server {

// CallbackUserdata and its derived classes are intended to be created by
// Server::MakeUserdata<T> and then passed as the userdata argument for Dawn
// callbacks.
// It contains a pointer back to the Server so that the callback can call the
// Server to perform operations like serialization, and it contains a weak pointer
// |serverIsAlive|. If the weak pointer has expired, it means the server has
// been destroyed and the callback must not use the Server pointer.
// To assist with checking |serverIsAlive| and lifetime management of the userdata,
// |ForwardToServerHelper| (defined earlier in this file) can be used to acquire the
// userdata, return early if |serverIsAlive| has expired, and then forward the arguments to
// userdata->server->MyCallbackHandler.
//
// Example Usage:
//
// struct MyUserdata : CallbackUserdata { uint32_t foo; };
//
// auto userdata = MakeUserdata<MyUserdata>();
// userdata->foo = 2;
//
// callMyCallbackHandler(
//      ForwardToServerHelper<&Server::MyCallbackHandler>::Callback,
//      userdata.release());
//
// void Server::MyCallbackHandler(MyUserdata* userdata, Other args) { }
struct CallbackUserdata {
    const std::weak_ptr<Server> server;
    const std::shared_ptr<const DawnProcTable> procs;

    CallbackUserdata() = delete;
    CallbackUserdata(const std::weak_ptr<Server>& server,
                     std::shared_ptr<const DawnProcTable>& procs);
};

struct MapUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle buffer;
    ObjectId instanceId;
    Future future;
    size_t offset;
    size_t size;
    wgpu::MapMode mode;
};

struct ErrorScopeUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle device;
    ObjectId instanceId;
    Future future;
};

struct ShaderModuleGetCompilationInfoUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectId instanceId;
    Future future;
};

struct QueueWorkDoneUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle queue;
    ObjectId instanceId;
    Future future;
};

struct CreatePipelineAsyncUserData : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectHandle device;
    ObjectId instanceId;
    Future future;
    ObjectHandle pipeline;
};

struct RequestAdapterUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectId instanceId;
    Future future;
    ObjectHandle adapter;
};

struct RequestDeviceUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectId instanceId;
    Future future;
    ObjectHandle device;
};

struct DeviceLostUserdata : CallbackUserdata {
    using CallbackUserdata::CallbackUserdata;

    ObjectId instanceId;
    Future future;
};

class Server : public ServerBase {
  public:
    static std::shared_ptr<Server> Create(const DawnProcTable& procs,
                                          CommandSerializer* serializer,
                                          MemoryTransferService* memoryTransferService,
                                          bool useSpontaneousCallbacks);
    ~Server() override;

    // ChunkedCommandHandler implementation
    bool HandleCommands(Span<const volatile std::byte> commands) override;

    WireResult InjectBuffer(WGPUBuffer buffer, const Handle& handle, const Handle& deviceHandle);
    WireResult InjectTexture(WGPUTexture texture, const Handle& handle, const Handle& deviceHandle);
    WireResult InjectSurface(WGPUSurface surface,
                             const Handle& handle,
                             const Handle& instanceHandle);
    WireResult InjectInstance(WGPUInstance instance, const Handle& handle);

    WGPUDevice GetDevice(uint32_t id, uint32_t generation);
    using ServerBase::IsDeviceKnown;

    // Flushes the command serialized from server->client if spontaneous callbacks are enabled.
    void Flush();

    template <typename T>
        requires(std::is_base_of_v<CallbackUserdata, T>)
    std::unique_ptr<T> MakeUserdata() {
        return std::unique_ptr<T>(new T(mSelf, mProcs));
    }

    template <typename CallbackInfo,
              auto F,
              wgpu::CallbackMode DefaultMode = wgpu::CallbackMode::AllowProcessEvents>
    CallbackInfo MakeCallbackInfo(typename detail::ForwardToServerHelper<F>::Userdata* userdata) {
        return {
            nullptr,
            ToAPI(mUseSpontaneousCallbacks ? wgpu::CallbackMode::AllowSpontaneous : DefaultMode),
            &detail::ForwardToServerHelper<F>::Callback, userdata, nullptr};
    }

  private:
    Server(const DawnProcTable& procs,
           CommandSerializer* serializer,
           MemoryTransferService* memoryTransferService,
           bool useSpontaneousCallbacks);

    template <typename Cmd>
    void SerializeCommand(Cmd&& cmd) {
        mSerializer->SerializeCommand(std::forward<Cmd>(cmd));
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(Cmd&& cmd, Extensions&&... es) {
        mSerializer->SerializeCommand(std::forward<Cmd>(cmd), std::forward<Extensions>(es)...);
    }

    template <typename Cmd, typename... Args>
    void SerializeCommand(Cmd&, Args&&...) = delete;

    // Wrapper RAII helper for structs with FreeMember calls.
    template <typename Struct>
    class FreeMembers : public Struct {
      public:
        using CType = std::remove_pointer_t<decltype(ToAPI(static_cast<Struct*>(nullptr)))>;

        explicit FreeMembers(std::shared_ptr<const DawnProcTable>& procs)
            : Struct{}, mProcs(procs) {}
        ~FreeMembers() {
            ((*mProcs).*WGPUTraits<CType>::FreeMembers)(*ToAPI(static_cast<Struct*>(this)));
        }

      private:
        std::shared_ptr<const DawnProcTable> mProcs;
    };

    void SetForwardingDeviceCallbacks(Known<WGPUDevice> device);

    // Async event callbacks:
    //   These callbacks are expected to be called while holding the server object lock via
    //   |GetGuard|, and should almost always be followed by a call to |Flush|. Note that
    //   pretty much all of these functions are called as a part of
    //   ForwardToServerHelper::Callback unless specified otherwise in the comments.
    void OnDeviceLost(DeviceLostUserdata* userdata,
                      WGPUDevice const* device,
                      wgpu::DeviceLostReason reason,
                      StringView message);
    void OnDevicePopErrorScope(ErrorScopeUserdata* userdata,
                               wgpu::PopErrorScopeStatus status,
                               wgpu::ErrorType type,
                               StringView message);
    void OnBufferMapAsyncCallback(MapUserdata* userdata,
                                  wgpu::MapAsyncStatus status,
                                  StringView message);
    void OnQueueWorkDone(QueueWorkDoneUserdata* userdata,
                         wgpu::QueueWorkDoneStatus status,
                         StringView message);
    void OnCreateComputePipelineAsyncCallback(CreatePipelineAsyncUserData* userdata,
                                              wgpu::CreatePipelineAsyncStatus status,
                                              WGPUComputePipeline pipeline,
                                              StringView message);
    void OnCreateRenderPipelineAsyncCallback(CreatePipelineAsyncUserData* userdata,
                                             wgpu::CreatePipelineAsyncStatus status,
                                             WGPURenderPipeline pipeline,
                                             StringView message);
    void OnShaderModuleGetCompilationInfo(ShaderModuleGetCompilationInfoUserdata* userdata,
                                          wgpu::CompilationInfoRequestStatus status,
                                          const CompilationInfo* info);
    void OnRequestAdapterCallback(RequestAdapterUserdata* userdata,
                                  wgpu::RequestAdapterStatus status,
                                  WGPUAdapter adapter,
                                  StringView message);
    void OnRequestDeviceCallback(RequestDeviceUserdata* userdata,
                                 wgpu::RequestDeviceStatus status,
                                 WGPUDevice device,
                                 StringView message);
    // The |OnUncapturedError| callback is special in that:
    //   1) It is a repeating callback, so it can't be used with
    //      ForwardToServerHelper::Callback.
    void OnUncapturedError(ObjectHandle device, wgpu::ErrorType type, StringView message);
    // The |OnLogging| callback is special in that:
    //   1) It is a repeating callback, so it can't be used with
    //      ForwardToServerHelper::Callback.
    //   2) It does not require holding the server object storage lock, i.e. |GetGuard| before
    //      being called because it never interacts with the object store.
    void OnLogging(ObjectHandle device, wgpu::LoggingType type, StringView message);

#include "dawn/wire/server/ServerPrototypes_autogen.inc"

    MutexProtected<ChunkedCommandSerializer> mSerializer;
    std::unique_ptr<MemoryTransferService> mOwnedMemoryTransferService = nullptr;
    raw_ptr<MemoryTransferService> mMemoryTransferService = nullptr;
    bool mUseSpontaneousCallbacks = false;

    // Weak pointer to self to facilitate creation of userdata.
    std::weak_ptr<Server> mSelf;
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService();

}  // namespace server
}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_SERVER_SERVER_H_
