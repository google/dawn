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

#ifndef SRC_DAWN_NATIVE_ENCODINGCONTEXT_H_
#define SRC_DAWN_NATIVE_ENCODINGCONTEXT_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/IndirectDrawMetadata.h"
#include "dawn/native/PassResourceUsageTracker.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class CommandEncoder;
class DeviceBase;
class ApiObjectBase;

// Base class for allocating/iterating commands.
// It performs error tracking as well as encoding state for render/compute passes.
class EncodingContext {
  public:
    EncodingContext(DeviceBase* device, const ApiObjectBase* initialEncoder);
    ~EncodingContext();

    // Marks the encoding context as destroyed so that any future encodes will fail, and all
    // encoded commands are released.
    void Destroy();

    CommandIterator AcquireCommands();
    CommandIterator* GetIterator();

    // Functions to handle encoder errors
    void HandleError(std::unique_ptr<ErrorData> error);

    inline bool ConsumedError(MaybeError maybeError) {
        if (DAWN_UNLIKELY(maybeError.IsError())) {
            HandleError(maybeError.AcquireError());
            return true;
        }
        return false;
    }

    template <typename... Args>
    inline bool ConsumedError(MaybeError maybeError, const char* formatStr, const Args&... args) {
        if (DAWN_UNLIKELY(maybeError.IsError())) {
            std::unique_ptr<ErrorData> error = maybeError.AcquireError();
            if (error->GetType() == InternalErrorType::Validation) {
                std::string out;
                absl::UntypedFormatSpec format(formatStr);
                if (absl::FormatUntyped(&out, format, {absl::FormatArg(args)...})) {
                    error->AppendContext(std::move(out));
                } else {
                    error->AppendContext(
                        absl::StrFormat("[Failed to format error message: \"%s\"].", formatStr));
                }
            }
            HandleError(std::move(error));
            return true;
        }
        return false;
    }

    inline bool CheckCurrentEncoder(const ApiObjectBase* encoder) {
        if (mDestroyed) {
            HandleError(DAWN_VALIDATION_ERROR("Recording in a destroyed %s.", mCurrentEncoder));
            return false;
        }
        if (DAWN_UNLIKELY(encoder != mCurrentEncoder)) {
            if (mCurrentEncoder != mTopLevelEncoder) {
                // The top level encoder was used when a pass encoder was current.
                HandleError(DAWN_VALIDATION_ERROR(
                    "Command cannot be recorded while %s is locked and %s is currently open.",
                    mTopLevelEncoder, mCurrentEncoder));
            } else {
                HandleError(DAWN_VALIDATION_ERROR("Recording in an error %s.", encoder));
            }
            return false;
        }
        return true;
    }

    template <typename EncodeFunction>
    inline bool TryEncode(const ApiObjectBase* encoder, EncodeFunction&& encodeFunction) {
        if (!CheckCurrentEncoder(encoder)) {
            return false;
        }
        ASSERT(!mWasMovedToIterator);
        return !ConsumedError(encodeFunction(&mPendingCommands));
    }

    template <typename EncodeFunction, typename... Args>
    inline bool TryEncode(const ApiObjectBase* encoder,
                          EncodeFunction&& encodeFunction,
                          const char* formatStr,
                          const Args&... args) {
        if (!CheckCurrentEncoder(encoder)) {
            return false;
        }
        ASSERT(!mWasMovedToIterator);
        return !ConsumedError(encodeFunction(&mPendingCommands), formatStr, args...);
    }

    // Must be called prior to encoding a BeginRenderPassCmd. Note that it's OK to call this
    // and then not actually call EnterPass+ExitRenderPass, for example if some other pass setup
    // failed validation before the BeginRenderPassCmd could be encoded.
    void WillBeginRenderPass();

    // Functions to set current encoder state
    void EnterPass(const ApiObjectBase* passEncoder);
    MaybeError ExitRenderPass(const ApiObjectBase* passEncoder,
                              RenderPassResourceUsageTracker usageTracker,
                              CommandEncoder* commandEncoder,
                              IndirectDrawMetadata indirectDrawMetadata);
    void ExitComputePass(const ApiObjectBase* passEncoder, ComputePassResourceUsage usages);
    MaybeError Finish();

    // Called when a pass encoder is deleted. Provides an opportunity to clean up if it's the
    // mCurrentEncoder.
    void EnsurePassExited(const ApiObjectBase* passEncoder);

    const RenderPassUsages& GetRenderPassUsages() const;
    const ComputePassUsages& GetComputePassUsages() const;
    RenderPassUsages AcquireRenderPassUsages();
    ComputePassUsages AcquireComputePassUsages();

    void PushDebugGroupLabel(const char* groupLabel);
    void PopDebugGroupLabel();

  private:
    void CommitCommands(CommandAllocator allocator);

    bool IsFinished() const;
    void MoveToIterator();

    DeviceBase* mDevice;

    // There can only be two levels of encoders. Top-level and render/compute pass.
    // The top level encoder is the encoder the EncodingContext is created with.
    // It doubles as flag to check if encoding has been Finished.
    const ApiObjectBase* mTopLevelEncoder;
    // The current encoder must be the same as the encoder provided to TryEncode,
    // otherwise an error is produced. It may be nullptr if the EncodingContext is an error.
    // The current encoder changes with Enter/ExitPass which should be called by
    // CommandEncoder::Begin/EndPass.
    const ApiObjectBase* mCurrentEncoder;

    RenderPassUsages mRenderPassUsages;
    bool mWereRenderPassUsagesAcquired = false;
    ComputePassUsages mComputePassUsages;
    bool mWereComputePassUsagesAcquired = false;

    CommandAllocator mPendingCommands;

    std::vector<CommandAllocator> mAllocators;
    CommandIterator mIterator;
    bool mWasMovedToIterator = false;
    bool mWereCommandsAcquired = false;
    bool mDestroyed = false;

    std::unique_ptr<ErrorData> mError;
    std::vector<std::string> mDebugGroupLabels;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ENCODINGCONTEXT_H_
