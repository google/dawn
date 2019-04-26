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

#include "tests/unittests/validation/ValidationTest.h"

namespace {

class ToggleValidationTest : public ValidationTest {
};

// Tests querying the detail of a toggle from dawn_native::InstanceBase works correctly.
TEST_F(ToggleValidationTest, QueryToggleInfo) {
    // Query with a valid toggle name
    {
        const char* kValidToggleName = "emulate_store_and_msaa_resolve";
        const dawn_native::ToggleInfo* toggleInfo = instance->GetToggleInfo(kValidToggleName);
        ASSERT_NE(nullptr, toggleInfo);
        ASSERT_NE(nullptr, toggleInfo->name);
        ASSERT_NE(nullptr, toggleInfo->description);
        ASSERT_NE(nullptr, toggleInfo->url);
    }

    // Query with an invalid toggle name
    {
        const char* kInvalidToggleName = "!@#$%^&*";
        const dawn_native::ToggleInfo* toggleInfo = instance->GetToggleInfo(kInvalidToggleName);
        ASSERT_EQ(nullptr, toggleInfo);
    }
}

// Tests overriding toggles when creating a device works correctly.
TEST_F(ToggleValidationTest, OverrideToggleUsage) {
    // Dawn unittests use null Adapters, so there should be no toggles that are enabled by default
    {
        std::vector<const char*> toggleNames = dawn_native::GetTogglesUsed(device.Get());
        ASSERT_EQ(0u, toggleNames.size());
    }

    // Create device with a valid name of a toggle
    {
        const char* kValidToggleName = "emulate_store_and_msaa_resolve";
        dawn_native::DeviceDescriptor descriptor;
        descriptor.forceEnabledToggles.push_back(kValidToggleName);

        DawnDevice deviceWithToggle = adapter.CreateDevice(&descriptor);
        std::vector<const char*> toggleNames = dawn_native::GetTogglesUsed(deviceWithToggle);
        ASSERT_EQ(1u, toggleNames.size());
        ASSERT_EQ(0, strcmp(kValidToggleName, toggleNames[0]));
    }

    // Create device with an invalid toggle name
    {
        dawn_native::DeviceDescriptor descriptor;
        descriptor.forceEnabledToggles.push_back("!@#$%^&*");

        DawnDevice deviceWithToggle = adapter.CreateDevice(&descriptor);
        std::vector<const char*> toggleNames = dawn_native::GetTogglesUsed(deviceWithToggle);
        ASSERT_EQ(0u, toggleNames.size());
    }
}
}  // anonymous namespace
