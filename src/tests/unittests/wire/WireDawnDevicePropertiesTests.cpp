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

#include "dawn_wire/Wire.h"
#include "gtest/gtest.h"

#include <vector>

class WireDawnDevicePropertiesTests : public testing::Test {};

// Test that the serialization and deserialization of DawnDeviceProperties can work correctly.
TEST_F(WireDawnDevicePropertiesTests, SerializeDawnDeviceProperties) {
    DawnDeviceProperties sentDawnDeviceProperties;
    sentDawnDeviceProperties.textureCompressionBC = true;

    size_t sentDawnDevicePropertiesSize =
        dawn_wire::SerializedWGPUDevicePropertiesSize(&sentDawnDeviceProperties);
    std::vector<char> buffer;
    buffer.resize(sentDawnDevicePropertiesSize);
    dawn_wire::SerializeWGPUDeviceProperties(&sentDawnDeviceProperties, buffer.data());

    DawnDeviceProperties receivedDawnDeviceProperties;
    dawn_wire::DeserializeWGPUDeviceProperties(&receivedDawnDeviceProperties, buffer.data());
    ASSERT_TRUE(receivedDawnDeviceProperties.textureCompressionBC);
}
