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

#include "common/WindowsUtils.h"

#include "common/windows_with_undefs.h"

#include <memory>

std::string WCharToUTF8(const wchar_t* input) {
    // The -1 argument asks WideCharToMultiByte to use the null terminator to know the size of
    // input. It will return a size that includes the null terminator.
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, input, -1, nullptr, 0, nullptr, nullptr);

    // When we can use C++17 this can be changed to use string.data() instead.
    std::unique_ptr<char[]> result = std::make_unique<char[]>(requiredSize);
    WideCharToMultiByte(CP_UTF8, 0, input, -1, result.get(), requiredSize, nullptr, nullptr);

    // This will allocate the returned std::string and then destroy result.
    return std::string(result.get(), result.get() + (requiredSize - 1));
}

std::wstring UTF8ToWStr(const char* input) {
    // The -1 argument asks MultiByteToWideChar to use the null terminator to know the size of
    // input. It will return a size that includes the null terminator.
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, input, -1, nullptr, 0);

    // When we can use C++17 this can be changed to use string.data() instead.
    std::unique_ptr<wchar_t[]> result = std::make_unique<wchar_t[]>(requiredSize);
    MultiByteToWideChar(CP_UTF8, 0, input, -1, result.get(), requiredSize);

    // This will allocate the returned std::string and then destroy result.
    return std::wstring(result.get(), result.get() + (requiredSize - 1));
}
