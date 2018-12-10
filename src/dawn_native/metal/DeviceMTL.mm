// Copyright 2018 The Dawn Authors
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

#include "dawn_native/metal/DeviceMTL.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/MetalBackend.h"
#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/metal/BlendStateMTL.h"
#include "dawn_native/metal/BufferMTL.h"
#include "dawn_native/metal/CommandBufferMTL.h"
#include "dawn_native/metal/ComputePipelineMTL.h"
#include "dawn_native/metal/DepthStencilStateMTL.h"
#include "dawn_native/metal/InputStateMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/QueueMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"
#include "dawn_native/metal/ResourceUploader.h"
#include "dawn_native/metal/SamplerMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/SwapChainMTL.h"
#include "dawn_native/metal/TextureMTL.h"

#include <IOKit/graphics/IOGraphicsLib.h>
#include <unistd.h>

namespace dawn_native { namespace metal {

    namespace {
        // Since CGDisplayIOServicePort was deprecated in macOS 10.9, we need create
        // an alternative function for getting I/O service port from current display.
        io_service_t GetDisplayIOServicePort() {
            // The matching service port (or 0 if none can be found)
            io_service_t servicePort = 0;

            // Create matching dictionary for display service
            CFMutableDictionaryRef matchingDict = IOServiceMatching("IODisplayConnect");
            if (matchingDict == nullptr) {
                return 0;
            }

            io_iterator_t iter;
            // IOServiceGetMatchingServices look up the default master ports that match a
            // matching dictionary, and will consume the reference on the matching dictionary,
            // so we don't need to release the dictionary, but the iterator handle should
            // be released when its iteration is finished.
            if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter) !=
                kIOReturnSuccess) {
                return 0;
            }

            // Vendor number and product number of current main display
            const uint32_t displayVendorNumber = CGDisplayVendorNumber(kCGDirectMainDisplay);
            const uint32_t displayProductNumber = CGDisplayModelNumber(kCGDirectMainDisplay);

            io_service_t serv;
            while ((serv = IOIteratorNext(iter)) != IO_OBJECT_NULL) {
                CFDictionaryRef displayInfo =
                    IODisplayCreateInfoDictionary(serv, kIODisplayOnlyPreferredName);

                CFNumberRef vendorIDRef, productIDRef;
                Boolean success;
                // The ownership of CF object follows the 'Get Rule', we don't need to
                // release these values
                success = CFDictionaryGetValueIfPresent(displayInfo, CFSTR(kDisplayVendorID),
                                                        (const void**)&vendorIDRef);
                success &= CFDictionaryGetValueIfPresent(displayInfo, CFSTR(kDisplayProductID),
                                                         (const void**)&productIDRef);
                if (success) {
                    CFIndex vendorID = 0, productID = 0;
                    CFNumberGetValue(vendorIDRef, kCFNumberSInt32Type, &vendorID);
                    CFNumberGetValue(productIDRef, kCFNumberSInt32Type, &productID);

                    if (vendorID == displayVendorNumber && productID == displayProductNumber) {
                        // Check if vendor id and product id match with current display's
                        // If it does, we find the desired service port
                        servicePort = serv;
                        CFRelease(displayInfo);
                        break;
                    }
                }

                CFRelease(displayInfo);
                IOObjectRelease(serv);
            }
            IOObjectRelease(iter);
            return servicePort;
        }

        // Get integer property from registry entry.
        uint32_t GetEntryProperty(io_registry_entry_t entry, CFStringRef name) {
            uint32_t value = 0;

            // Recursively search registry entry and its parents for property name
            // The data should release with CFRelease
            CFDataRef data = static_cast<CFDataRef>(IORegistryEntrySearchCFProperty(
                entry, kIOServicePlane, name, kCFAllocatorDefault,
                kIORegistryIterateRecursively | kIORegistryIterateParents));

            if (data != nullptr) {
                const uint32_t* valuePtr =
                    reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(data));
                if (valuePtr) {
                    value = *valuePtr;
                }

                CFRelease(data);
            }

            return value;
        }
    }  // anonymous namespace

    dawnDevice CreateDevice(id<MTLDevice> metalDevice) {
        return reinterpret_cast<dawnDevice>(new Device(metalDevice));
    }

    // Device

    Device::Device(id<MTLDevice> mtlDevice)
        : mMtlDevice(mtlDevice),
          mMapTracker(new MapRequestTracker(this)),
          mResourceUploader(new ResourceUploader(this)) {
        [mMtlDevice retain];
        mCommandQueue = [mMtlDevice newCommandQueue];
        CollectPCIInfo();
    }

    Device::~Device() {
        // Wait for all commands to be finished so we can free resources SubmitPendingCommandBuffer
        // may not increment the pendingCommandSerial if there are no pending commands, so we can't
        // store the pendingSerial before SubmitPendingCommandBuffer then wait for it to be passed.
        // Instead we submit and wait for the serial before the next pendingCommandSerial.
        SubmitPendingCommandBuffer();
        while (mCompletedSerial != mLastSubmittedSerial) {
            usleep(100);
        }
        Tick();

        [mPendingCommands release];
        mPendingCommands = nil;

        mMapTracker = nullptr;
        mResourceUploader = nullptr;

        [mMtlDevice release];
        mMtlDevice = nil;

        [mCommandQueue release];
        mCommandQueue = nil;
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return new BindGroup(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return new RenderPipeline(this, descriptor);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        return new ShaderModule(this, descriptor);
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    Serial Device::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    Serial Device::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    Serial Device::GetPendingCommandSerial() const {
        return mLastSubmittedSerial + 1;
    }

    void Device::TickImpl() {
        mResourceUploader->Tick(mCompletedSerial);
        mMapTracker->Tick(mCompletedSerial);

        if (mPendingCommands != nil) {
            SubmitPendingCommandBuffer();
        } else if (mCompletedSerial == mLastSubmittedSerial) {
            // If there's no GPU work in flight we still need to artificially increment the serial
            // so that CPU operations waiting on GPU completion can know they don't have to wait.
            mCompletedSerial++;
            mLastSubmittedSerial++;
        }
    }

    const dawn_native::PCIInfo& Device::GetPCIInfo() const {
        return mPCIInfo;
    }

    id<MTLDevice> Device::GetMTLDevice() {
        return mMtlDevice;
    }

    id<MTLCommandBuffer> Device::GetPendingCommandBuffer() {
        if (mPendingCommands == nil) {
            mPendingCommands = [mCommandQueue commandBuffer];
            [mPendingCommands retain];
        }
        return mPendingCommands;
    }

    void Device::SubmitPendingCommandBuffer() {
        if (mPendingCommands == nil) {
            return;
        }

        // Ok, ObjC blocks are weird. My understanding is that local variables are captured by value
        // so this-> works as expected. However it is unclear how members are captured, (are they
        // captured using this-> or by value?) so we make a copy of the pendingCommandSerial on the
        // stack.
        mLastSubmittedSerial++;
        Serial pendingSerial = mLastSubmittedSerial;
        [mPendingCommands addCompletedHandler:^(id<MTLCommandBuffer>) {
            this->mCompletedSerial = pendingSerial;
        }];

        [mPendingCommands commit];
        [mPendingCommands release];
        mPendingCommands = nil;
    }

    MapRequestTracker* Device::GetMapTracker() const {
        return mMapTracker.get();
    }

    ResourceUploader* Device::GetResourceUploader() const {
        return mResourceUploader.get();
    }

    void Device::CollectPCIInfo() {
        io_registry_entry_t entry = GetDisplayIOServicePort();
        if (entry != IO_OBJECT_NULL) {
            mPCIInfo.vendorId = GetEntryProperty(entry, CFSTR("vendor-id"));
            mPCIInfo.deviceId = GetEntryProperty(entry, CFSTR("device-id"));
            IOObjectRelease(entry);
        }

        mPCIInfo.name = std::string([mMtlDevice.name UTF8String]);
    }

}}  // namespace dawn_native::metal
