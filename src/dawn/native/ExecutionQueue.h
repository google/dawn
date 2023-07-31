// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_EXECUTIONQUEUE_H_
#define SRC_DAWN_NATIVE_EXECUTIONQUEUE_H_

#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native {

// Represents an engine which processes a stream of GPU work. It handles the tracking and
// update of the various ExecutionSerials related to that work.
class ExecutionQueueBase {
  public:
    // The latest serial known to have completed execution on the queue.
    ExecutionSerial GetCompletedCommandSerial() const;
    // The serial of the latest batch of work sent for execution.
    ExecutionSerial GetLastSubmittedCommandSerial() const;
    // The serial of the batch that is currently pending submission.
    ExecutionSerial GetPendingCommandSerial() const;
    // The serial by which time all currently submitted or pending operations will be completed.
    ExecutionSerial GetScheduledWorkDoneSerial() const;
    // Whether the execution queue has scheduled commands to be submitted or executing.
    bool HasScheduledCommands() const;

    // Check for passed fences and set the new completed serial.
    MaybeError CheckPassedSerials();

    // For the commands being internally recorded in backend, that were not urgent to submit, this
    // method makes them to be submitted as soon as possible in next ticks.
    virtual void ForceEventualFlushOfCommands() = 0;

    // During shut down of device, some operations might have been started since the last submit
    // and waiting on a serial that doesn't have a corresponding fence enqueued. Fake serials to
    // make all commands look completed.
    void AssumeCommandsComplete();

  protected:
    // Increment mLastSubmittedSerial when we submit the next serial
    void IncrementLastSubmittedCommandSerial();

    // WaitForIdleForDestruction waits for GPU to finish, checks errors and gets ready for
    // destruction. This is only used when properly destructing the device. For a real
    // device loss, this function doesn't need to be called since the driver already closed all
    // resources.
    virtual MaybeError WaitForIdleForDestruction() = 0;

  private:
    // Each backend should implement to check their passed fences if there are any and return a
    // completed serial. Return 0 should indicate no fences to check.
    virtual ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() = 0;
    // mCompletedSerial tracks the last completed command serial that the fence has returned.
    // mLastSubmittedSerial tracks the last submitted command serial.
    // During device removal, the serials could be artificially incremented
    // to make it appear as if commands have been compeleted.
    ExecutionSerial mCompletedSerial = ExecutionSerial(0);
    ExecutionSerial mLastSubmittedSerial = ExecutionSerial(0);

    // Indicates whether the backend has pending commands to be submitted as soon as possible.
    virtual bool HasPendingCommands() const = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EXECUTIONQUEUE_H_
