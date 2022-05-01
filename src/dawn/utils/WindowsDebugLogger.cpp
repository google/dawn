// Copyright 2020 The Dawn Authors
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

#include <array>
#include <thread>

#include "dawn/utils/PlatformDebugLogger.h"

#include "dawn/common/Assert.h"
#include "dawn/common/windows_with_undefs.h"

namespace utils {

class WindowsDebugLogger : public PlatformDebugLogger {
  public:
    WindowsDebugLogger() : PlatformDebugLogger() {
        if (IsDebuggerPresent()) {
            // This condition is true when running inside Visual Studio or some other debugger.
            // Messages are already printed there so we don't need to do anything.
            return;
        }

        mShouldExitHandle = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        ASSERT(mShouldExitHandle != nullptr);

        mThread = std::thread(
            [](HANDLE shouldExit) {
                // https://blogs.msdn.microsoft.com/reiley/2011/07/29/a-debugging-approach-to-outputdebugstring/
                // for the layout of this struct.
                struct {
                    DWORD process_id;
                    char data[4096 - sizeof(DWORD)];
                }* dbWinBuffer = nullptr;

                HANDLE file = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                                 sizeof(*dbWinBuffer), "DBWIN_BUFFER");
                ASSERT(file != nullptr);
                ASSERT(file != INVALID_HANDLE_VALUE);

                dbWinBuffer = static_cast<decltype(dbWinBuffer)>(
                    MapViewOfFile(file, SECTION_MAP_READ, 0, 0, 0));
                ASSERT(dbWinBuffer != nullptr);

                HANDLE dbWinBufferReady = CreateEventA(nullptr, FALSE, FALSE, "DBWIN_BUFFER_READY");
                ASSERT(dbWinBufferReady != nullptr);

                HANDLE dbWinDataReady = CreateEventA(nullptr, FALSE, FALSE, "DBWIN_DATA_READY");
                ASSERT(dbWinDataReady != nullptr);

                std::array<HANDLE, 2> waitHandles = {shouldExit, dbWinDataReady};
                while (true) {
                    SetEvent(dbWinBufferReady);
                    DWORD wait = WaitForMultipleObjects(waitHandles.size(), waitHandles.data(),
                                                        FALSE, INFINITE);
                    if (wait == WAIT_OBJECT_0) {
                        break;
                    }
                    ASSERT(wait == WAIT_OBJECT_0 + 1);
                    fprintf(stderr, "%.*s\n", static_cast<int>(sizeof(dbWinBuffer->data)),
                            dbWinBuffer->data);
                    fflush(stderr);
                }

                CloseHandle(dbWinDataReady);
                CloseHandle(dbWinBufferReady);
                UnmapViewOfFile(dbWinBuffer);
                CloseHandle(file);
            },
            mShouldExitHandle);
    }

    ~WindowsDebugLogger() override {
        if (IsDebuggerPresent()) {
            // This condition is true when running inside Visual Studio or some other debugger.
            // Messages are already printed there so we don't need to do anything.
            return;
        }

        if (mShouldExitHandle != nullptr) {
            BOOL result = SetEvent(mShouldExitHandle);
            ASSERT(result != 0);
            CloseHandle(mShouldExitHandle);
        }

        if (mThread.joinable()) {
            mThread.join();
        }
    }

  private:
    std::thread mThread;
    HANDLE mShouldExitHandle = INVALID_HANDLE_VALUE;
};

PlatformDebugLogger* CreatePlatformDebugLogger() {
    return new WindowsDebugLogger();
}

}  // namespace utils
