// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_DYNAMICLIB_H_
#define SRC_DAWN_COMMON_DYNAMICLIB_H_

#include <string>
#include <type_traits>

#include "dawn/common/Assert.h"

class DynamicLib {
  public:
    DynamicLib() = default;
    ~DynamicLib();

    DynamicLib(const DynamicLib&) = delete;
    DynamicLib& operator=(const DynamicLib&) = delete;

    DynamicLib(DynamicLib&& other);
    DynamicLib& operator=(DynamicLib&& other);

    bool Valid() const;

    bool Open(const std::string& filename, std::string* error = nullptr);
    void Close();

    void* GetProc(const std::string& procName, std::string* error = nullptr) const;

    template <typename T>
    bool GetProc(T** proc, const std::string& procName, std::string* error = nullptr) const {
        ASSERT(proc != nullptr);
        static_assert(std::is_function<T>::value);

        *proc = reinterpret_cast<T*>(GetProc(procName, error));
        return *proc != nullptr;
    }

  private:
    void* mHandle = nullptr;
};

#endif  // SRC_DAWN_COMMON_DYNAMICLIB_H_
