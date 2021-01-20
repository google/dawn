// Copyright 2021 The Dawn Authors
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

#ifndef COMMON_WORKERTHREAD_H_
#define COMMON_WORKERTHREAD_H_

#include "common/NonCopyable.h"
#include "dawn_platform/DawnPlatform.h"

class AsyncWorkerThreadPool : public dawn_platform::WorkerTaskPool, public NonCopyable {
  public:
    std::unique_ptr<dawn_platform::WaitableEvent> PostWorkerTask(
        dawn_platform::PostWorkerTaskCallback callback,
        void* userdata) override;
};

#endif
