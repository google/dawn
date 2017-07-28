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

#ifndef TESTS_UNITTESTS_VALIDATIONTEST_H_
#define TESTS_UNITTESTS_VALIDATIONTEST_H_

#include "gtest/gtest.h"
#include "nxt/nxtcpp.h"
#include "nxt/nxtcpp_traits.h"

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
        //     nxt::Foo foo = AssertWillBe[Success|Error](device.CreateFooBuilder(), "my foo")
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

        void CreateSimpleRenderPassAndFramebuffer(const nxt::Device& device, nxt::RenderPass* renderpass, nxt::Framebuffer* framebuffer);

        // Helper functions to create objects to test validation.

        struct DummyRenderPass {
            nxt::RenderPass renderPass;
            nxt::Framebuffer framebuffer;
            nxt::Texture attachment;
            nxt::TextureFormat attachmentFormat;
            uint32_t width;
            uint32_t height;
        };
        DummyRenderPass CreateDummyRenderPass();

    protected:
        nxt::Device device;

    private:
        static void OnDeviceError(const char* message, nxtCallbackUserdata userdata);
        bool expectError = false;
        bool error = false;

        struct BuilderStatusExpectations {
            bool expectSuccess;
            std::string debugName;

            bool gotStatus = false;
            std::string statusMessage;
            nxtBuilderErrorStatus status;
        };
        std::vector<BuilderStatusExpectations> expectations;

        template<typename Builder>
        Builder AddExpectation(Builder& builder, std::string debugName, bool expectSuccess);

        static void OnBuilderErrorStatus(nxtBuilderErrorStatus status, const char* message, nxt::CallbackUserdata userdata1, nxt::CallbackUserdata userdata2);
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
    uint64_t userdata2 = expectations.size();
    builder.SetErrorCallback(OnBuilderErrorStatus, userdata1, userdata2);

    expectations.emplace_back();
    auto& expectation = expectations.back();
    expectation.expectSuccess = expectSuccess;
    expectation.debugName = debugName;

    return std::move(builder);
}

#endif // TESTS_UNITTESTS_VALIDATIONTEST_H_
