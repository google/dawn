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

#include "dawn/common/WindowsUtils.h"

#include <memory>

#include "dawn/common/windows_with_undefs.h"

std::string WCharToUTF8(const wchar_t* input) {
    // The -1 argument asks WideCharToMultiByte to use the null terminator to know the size of
    // input. It will return a size that includes the null terminator.
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, input, -1, nullptr, 0, nullptr, nullptr);

    std::string result;
    result.resize(requiredSize - 1);
    WideCharToMultiByte(CP_UTF8, 0, input, -1, result.data(), requiredSize, nullptr, nullptr);

    return result;
}

std::wstring UTF8ToWStr(const char* input) {
    // The -1 argument asks MultiByteToWideChar to use the null terminator to know the size of
    // input. It will return a size that includes the null terminator.
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, input, -1, nullptr, 0);

    std::wstring result;
    result.resize(requiredSize - 1);
    MultiByteToWideChar(CP_UTF8, 0, input, -1, result.data(), requiredSize);

    return result;
}
