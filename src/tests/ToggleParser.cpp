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

#include "tests/ToggleParser.h"

#include <cstring>
#include <sstream>

ToggleParser::ToggleParser() = default;
ToggleParser::~ToggleParser() = default;

bool ToggleParser::ParseEnabledToggles(char* arg) {
    constexpr const char kEnableTogglesSwitch[] = "--enable-toggles=";
    size_t argLen = sizeof(kEnableTogglesSwitch) - 1;
    if (strncmp(arg, kEnableTogglesSwitch, argLen) == 0) {
        std::string toggle;
        std::stringstream toggles(arg + argLen);
        while (getline(toggles, toggle, ',')) {
            mEnabledToggles.push_back(toggle);
        }
        return true;
    }
    return false;
}

bool ToggleParser::ParseDisabledToggles(char* arg) {
    constexpr const char kDisableTogglesSwitch[] = "--disable-toggles=";
    size_t argLDis = sizeof(kDisableTogglesSwitch) - 1;
    if (strncmp(arg, kDisableTogglesSwitch, argLDis) == 0) {
        std::string toggle;
        std::stringstream toggles(arg + argLDis);
        while (getline(toggles, toggle, ',')) {
            mDisabledToggles.push_back(toggle);
        }
        return true;
    }
    return false;
}

const std::vector<std::string>& ToggleParser::GetEnabledToggles() const {
    return mEnabledToggles;
}

const std::vector<std::string>& ToggleParser::GetDisabledToggles() const {
    return mDisabledToggles;
}
