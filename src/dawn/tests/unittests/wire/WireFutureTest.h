// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_UNITTESTS_WIRE_WIREFUTURETEST_H_
#define SRC_DAWN_TESTS_UNITTESTS_WIRE_WIREFUTURETEST_H_

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/FutureUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/ParamGenerator.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireServer.h"

#include "gtest/gtest.h"

namespace dawn::wire {

enum class CallbackMode {
    Async,  // Legacy mode that internally defers to Spontaneous.
    WaitAny,
    ProcessEvents,
    Spontaneous,
};
WGPUCallbackMode ToWGPUCallbackMode(CallbackMode callbackMode);

struct WireFutureTestParam {
    CallbackMode mCallbackMode;
};
std::ostream& operator<<(std::ostream& os, const WireFutureTestParam& param);
static constexpr std::array kCallbackModes = {WireFutureTestParam{CallbackMode::Async},
                                              WireFutureTestParam{CallbackMode::WaitAny},
                                              WireFutureTestParam{CallbackMode::ProcessEvents},
                                              WireFutureTestParam{CallbackMode::Spontaneous}};

template <typename Param, typename... Params>
auto MakeParamGenerator(std::initializer_list<Params>&&... params) {
    return ParamGenerator<Param, WireFutureTestParam, Params...>(
        std::vector<WireFutureTestParam>(kCallbackModes.begin(), kCallbackModes.end()),
        std::forward<std::initializer_list<Params>&&>(params)...);
}

// Usage: DAWN_WIRE_FUTURE_TEST_PARAM_STRUCT(Foo, TypeA, TypeB, ...)
// Generate a test param struct called Foo which extends WireFutureTestParam and generated
// struct _Dawn_Foo. _Dawn_Foo has members of types TypeA, TypeB, etc. which are named mTypeA,
// mTypeB, etc. in the order they are placed in the macro argument list. Struct Foo should be
// constructed with an WireFutureTestParam as the first argument, followed by a list of values
// to initialize the base _Dawn_Foo struct.
// It is recommended to use alias declarations so that stringified types are more readable.
// Example:
//   using MyParam = unsigned int;
//   DAWN_WIRE_FUTURE_TEST_PARAM_STRUCT(FooParams, MyParam);
#define DAWN_WIRE_FUTURE_TEST_PARAM_STRUCT(StructName, ...) \
    DAWN_TEST_PARAM_STRUCT_BASE(WireFutureTestParam, StructName, __VA_ARGS__)

#define DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(testName, ...)                                   \
    INSTANTIATE_TEST_SUITE_P(                                                                \
        , testName, testing::ValuesIn(MakeParamGenerator<testName::ParamType>(__VA_ARGS__)), \
        &TestParamToString<testName::ParamType>)

template <typename Callback,
          typename CallbackInfo,
          auto& AsyncF,
          auto& FutureF,
          typename Params = WireFutureTestParam,
          typename AsyncFT = decltype(AsyncF),
          typename FutureFT = decltype(FutureF)>
class WireFutureTestWithParams : public WireTest, public testing::WithParamInterface<Params> {
  protected:
    using testing::WithParamInterface<Params>::GetParam;

    void SetUp() override {
        WireTest::SetUp();

        auto reservation = GetWireClient()->ReserveInstance();
        instance = reservation.instance;

        apiInstance = api.GetNewInstance();
        EXPECT_CALL(api, InstanceReference(apiInstance));
        EXPECT_TRUE(
            GetWireServer()->InjectInstance(apiInstance, reservation.id, reservation.generation));
    }

    void TearDown() override { WireTest::TearDown(); }

    // Calls the actual API that the test suite is exercising given the callback mode. This should
    // be used in favor of directly calling the API because the Async mode actually calls a
    // different entry point.
    template <typename... Args>
    void CallImpl(void* userdata, Args&&... args) {
        if (GetParam().mCallbackMode == CallbackMode::Async) {
            mAsyncF(std::forward<Args>(args)..., mMockCb.Callback(),
                    mMockCb.MakeUserdata(userdata));
        } else {
            CallbackInfo callbackInfo = {};
            callbackInfo.mode = ToWGPUCallbackMode(GetParam().mCallbackMode);
            callbackInfo.callback = mMockCb.Callback();
            callbackInfo.userdata = mMockCb.MakeUserdata(userdata);
            mFutureIDs.push_back(mFutureF(std::forward<Args>(args)..., callbackInfo).id);
        }
    }

    // Events are considered spontaneous if either we are using the legacy Async or the new
    // Spontaneous modes.
    bool IsSpontaneous() {
        CallbackMode callbackMode = GetParam().mCallbackMode;
        return callbackMode == CallbackMode::Async || callbackMode == CallbackMode::Spontaneous;
    }

    // In order to tightly bound when callbacks are expected to occur, test writers only have access
    // to the mock callback via the argument passed usually via a lamdba. The 'exp' lambda should
    // generally be a block of expectations on the mock callback followed by one statement where we
    // expect the callbacks to be called from. If the callbacks do not occur in the scope of the
    // lambda, the mock will fail the test.
    //
    // Usage:
    //   ExpectWireCallbackWhen([&](auto& mockCb) {
    //       // Set scoped expectations on the mock callback.
    //       EXPECT_CALL(mockCb, Call).Times(1);
    //
    //       // Call the statement where we want to ensure the callbacks occur.
    //       FlushCallbacks();
    //   });
    void ExpectWireCallbacksWhen(std::function<void(testing::MockCallback<Callback>&)> exp) {
        exp(mMockCb);
        ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mMockCb));
    }

    // Future suite adds the following flush mechanics for test writers so that they can have fine
    // grained control over when expectations should be set and verified.
    //
    // FlushFutures ensures that all futures become ready regardless of callback mode, while
    // FlushCallbacks ensures that all callbacks that were ready have been called. In most cases,
    // the intended use-case would look as follows:
    //
    //     // Call the API under test
    //     CallImpl(mockCb, this, args...);
    //     EXPECT_CALL(api, OnAsyncAPI(...)).WillOnce(InvokeWithoutArgs([&] {
    //         api.CallAsyncAPICallback(...);
    //     }));
    //
    //     FlushClient();
    //     FlushFutures(); // Ensures that the callbacks are ready (if applicable), but NOT called.
    //     EXPECT_CALL(mockCb, Call(...));
    //     FlushCallbacks();  // Calls the callbacks
    //
    // Note that in the example above we don't explicitly every call FlushServer and in most cases
    // that is probably the way to go because for Async and Spontaneous events, FlushServer will
    // actually trigger the callback. So instead, it is likely that the intention is instead to
    // break the calls into FlushFutures and FlushCallbacks for more control.
    void FlushFutures() {
        // For non-spontaneous callback modes, we need the flush the server in order for
        // the futures to become ready. For spontaneous modes, however, we don't flush the
        // server yet because that would also trigger the callback immediately.
        if (!IsSpontaneous()) {
            WireTest::FlushServer();
        }
    }
    void FlushCallbacks() {
        // Flushing the server will cause Async and Spontaneous callbacks to trigger right away.
        WireTest::FlushServer();

        CallbackMode callbackMode = GetParam().mCallbackMode;
        if (callbackMode == CallbackMode::WaitAny) {
            if (mFutureIDs.empty()) {
                return;
            }
            std::vector<WGPUFutureWaitInfo> waitInfos;
            for (auto futureID : mFutureIDs) {
                waitInfos.push_back({{futureID}, false});
            }
            EXPECT_EQ(wgpuInstanceWaitAny(instance, mFutureIDs.size(), waitInfos.data(), 0),
                      WGPUWaitStatus_Success);
        } else if (callbackMode == CallbackMode::ProcessEvents) {
            wgpuInstanceProcessEvents(instance);
        }
    }

    WGPUInstance instance;
    WGPUInstance apiInstance;

  private:
    AsyncFT mAsyncF = AsyncF;
    FutureFT mFutureF = FutureF;
    std::vector<FutureID> mFutureIDs;

    testing::MockCallback<Callback> mMockCb;
};

template <typename Callback, typename CallbackInfo, auto& AsyncF, auto& FutureF>
using WireFutureTest = WireFutureTestWithParams<Callback, CallbackInfo, AsyncF, FutureF>;

}  // namespace dawn::wire

#endif  // SRC_DAWN_TESTS_UNITTESTS_WIRE_WIREFUTURETEST_H_
