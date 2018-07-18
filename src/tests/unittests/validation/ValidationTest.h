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

#ifndef TESTS_UNITTESTS_VALIDATIONTEST_H_
#define TESTS_UNITTESTS_VALIDATIONTEST_H_

#include "gtest/gtest.h"
#include "dawn/dawncpp.h"
#include "dawn/dawncpp_traits.h"

#define ASSERT_DEVICE_ERROR(statement) \
    StartExpectDeviceError(); \
    statement; \
    ASSERT_TRUE(EndExpectDeviceError());

class ValidationTest : public testing::Test {
    public:
        ValidationTest();
        ~ValidationTest();

        void TearDown() override;

        // Use these methods to add expectations on the validation of a builder. The expectations are
        // checked on test teardown. Adding an expectation is done like the following:
        //
        //     dawn::Foo foo = AssertWillBe[Success|Error](device.CreateFooBuilder(), "my foo")
        //         .SetBar(1)
        //         .GetResult();
        //
        // The string argument is optional but will be printed when an expectations is missed, this
        // will help debug tests where multiple expectations are added.
        template<typename Builder>
        Builder AssertWillBeSuccess(Builder builder, std::string debugName = "");
        template<typename Builder>
        Builder AssertWillBeError(Builder builder, std::string debugName = "");

        void StartExpectDeviceError();
        bool EndExpectDeviceError();

        dawn::RenderPassDescriptor CreateSimpleRenderPass();

        // Helper functions to create objects to test validation.

        struct DummyRenderPass {
            dawn::RenderPassDescriptor renderPass;
            dawn::Texture attachment;
            dawn::TextureFormat attachmentFormat;
            uint32_t width;
            uint32_t height;
        };
        DummyRenderPass CreateDummyRenderPass();

    protected:
        dawn::Device device;

    private:
        static void OnDeviceError(const char* message, nxtCallbackUserdata userdata);
        bool mExpectError = false;
        bool mError = false;

        struct BuilderStatusExpectations {
            bool expectSuccess;
            std::string debugName;

            bool gotStatus = false;
            std::string statusMessage;
            nxtBuilderErrorStatus status;
        };
        std::vector<BuilderStatusExpectations> mExpectations;

        template<typename Builder>
        Builder AddExpectation(Builder& builder, std::string debugName, bool expectSuccess);

        static void OnBuilderErrorStatus(nxtBuilderErrorStatus status, const char* message, dawn::CallbackUserdata userdata1, dawn::CallbackUserdata userdata2);
};

// Template implementation details

template<typename Builder>
Builder ValidationTest::AssertWillBeSuccess(Builder builder, std::string debugName) {
    return AddExpectation(builder, debugName, true);
}

template<typename Builder>
Builder ValidationTest::AssertWillBeError(Builder builder, std::string debugName) {
    return AddExpectation(builder, debugName, false);
}

template<typename Builder>
Builder ValidationTest::AddExpectation(Builder& builder, std::string debugName, bool expectSuccess) {
    uint64_t userdata1 = reinterpret_cast<uintptr_t>(this);
    uint64_t userdata2 = mExpectations.size();
    builder.SetErrorCallback(OnBuilderErrorStatus, userdata1, userdata2);

    mExpectations.emplace_back();
    auto& expectation = mExpectations.back();
    expectation.expectSuccess = expectSuccess;
    expectation.debugName = debugName;

    return std::move(builder);
}

#endif // TESTS_UNITTESTS_VALIDATIONTEST_H_
