// Copyright 2017 The NXT Authors
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

#include "common/DynamicLib.h"

#include "common/Platform.h"

#if NXT_PLATFORM_WINDOWS
    #include <windows.h>
#elif NXT_PLATFORM_POSIX
    #include <dlfcn.h>
#else
    #error "Unsupported platform for DynamicLib"
#endif

DynamicLib::~DynamicLib() {
    Close();
}

DynamicLib::DynamicLib(DynamicLib&& other) {
    std::swap(this->handle, other.handle);
}

DynamicLib& DynamicLib::operator=(DynamicLib&& other) {
    std::swap(this->handle, other.handle);
    return *this;
}

bool DynamicLib::Valid() const {
    return handle != nullptr;
}

bool DynamicLib::Open(const std::string& filename, std::string* error) {
    #if NXT_PLATFORM_WINDOWS
        handle = LoadLibraryA(filename.c_str());

        if (handle == nullptr && error != nullptr) {
            *error = "Windows Error: " + std::to_string(GetLastError());
        }
    #elif NXT_PLATFORM_POSIX
        handle = dlopen(filename.c_str(), RTLD_NOW);

        if (handle == nullptr && error != nullptr) {
            *error = dlerror();
        }
    #else
        #error "Unsupported platform for DynamicLib"
    #endif

    return handle != nullptr;
}

void DynamicLib::Close() {
    if (handle == nullptr) {
        return;
    }

    #if NXT_PLATFORM_WINDOWS
        FreeLibrary(static_cast<HMODULE>(handle));
    #elif NXT_PLATFORM_POSIX
        dlclose(handle);
    #else
        #error "Unsupported platform for DynamicLib"
    #endif

    handle = nullptr;
}

void* DynamicLib::GetProc(const std::string& procName, std::string* error) const {
    void* proc = nullptr;

    #if NXT_PLATFORM_WINDOWS
        proc = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), procName.c_str()));

        if (proc == nullptr && error != nullptr) {
            *error = "Windows Error: " + std::to_string(GetLastError());
        }
    #elif NXT_PLATFORM_POSIX
        proc = reinterpret_cast<void*>(dlsym(handle, procName.c_str()));

        if (proc == nullptr && error != nullptr) {
            *error = dlerror();
        }
    #else
        #error "Unsupported platform for DynamicLib"
    #endif

    return proc;
}
