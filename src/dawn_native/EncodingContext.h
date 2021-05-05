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

#ifndef DAWNNATIVE_ENCODINGCONTEXT_H_
#define DAWNNATIVE_ENCODINGCONTEXT_H_

#include "dawn_native/CommandAllocator.h"
#include "dawn_native/Error.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/PassResourceUsageTracker.h"
#include "dawn_native/dawn_platform.h"

#include <string>

namespace dawn_native {

    class DeviceBase;
    class ObjectBase;

    // Base class for allocating/iterating commands.
    // It performs error tracking as well as encoding state for render/compute passes.
    class EncodingContext {
      public:
        EncodingContext(DeviceBase* device, const ObjectBase* initialEncoder);
        ~EncodingContext();

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

        inline bool CheckCurrentEncoder(const ObjectBase* encoder) {
            if (DAWN_UNLIKELY(encoder != mCurrentEncoder)) {
                if (mCurrentEncoder != mTopLevelEncoder) {
                    // The top level encoder was used when a pass encoder was current.
                    HandleError(DAWN_VALIDATION_ERROR("Command cannot be recorded inside a pass"));
                } else {
                    HandleError(DAWN_VALIDATION_ERROR(
                        "Recording in an error or already ended pass encoder"));
                }
                return false;
            }
            return true;
        }

        template <typename EncodeFunction>
        inline bool TryEncode(const ObjectBase* encoder, EncodeFunction&& encodeFunction) {
            if (!CheckCurrentEncoder(encoder)) {
                return false;
            }
            ASSERT(!mWasMovedToIterator);
            return !ConsumedError(encodeFunction(&mAllocator));
        }

        // Functions to set current encoder state
        void EnterPass(const ObjectBase* passEncoder);
        void ExitPass(const ObjectBase* passEncoder, RenderPassResourceUsage usages);
        void ExitPass(const ObjectBase* passEncoder, ComputePassResourceUsage usages);
        MaybeError Finish();

        const RenderPassUsages& GetRenderPassUsages() const;
        const ComputePassUsages& GetComputePassUsages() const;
        RenderPassUsages AcquireRenderPassUsages();
        ComputePassUsages AcquireComputePassUsages();

      private:
        bool IsFinished() const;
        void MoveToIterator();

        DeviceBase* mDevice;

        // There can only be two levels of encoders. Top-level and render/compute pass.
        // The top level encoder is the encoder the EncodingContext is created with.
        // It doubles as flag to check if encoding has been Finished.
        const ObjectBase* mTopLevelEncoder;
        // The current encoder must be the same as the encoder provided to TryEncode,
        // otherwise an error is produced. It may be nullptr if the EncodingContext is an error.
        // The current encoder changes with Enter/ExitPass which should be called by
        // CommandEncoder::Begin/EndPass.
        const ObjectBase* mCurrentEncoder;

        RenderPassUsages mRenderPassUsages;
        bool mWereRenderPassUsagesAcquired = false;
        ComputePassUsages mComputePassUsages;
        bool mWereComputePassUsagesAcquired = false;

        CommandAllocator mAllocator;
        CommandIterator mIterator;
        bool mWasMovedToIterator = false;
        bool mWereCommandsAcquired = false;

        std::unique_ptr<ErrorData> mError;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_ENCODINGCONTEXT_H_
