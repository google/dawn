// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <gtest/gtest.h>

#include <array>
#include <cfloat>
#include <climits>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wunique-object-duplication"
#pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
#include <v8.h>
#pragma clang diagnostic pop

#include "libplatform/libplatform.h"
#include "src/dawn/node/napi_v8/napi_v8.h"

namespace {

// Test fixture providing an isolated V8 environment and napi_env for each test case.
class NapiV8Test : public ::testing::Test {
  protected:
    static void SetUpTestSuite() {
        platform_ = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();
    }

    static void TearDownTestSuite() {
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
    }

    void SetUp() override {
        allocator_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
        create_params_.array_buffer_allocator = allocator_.get();
        isolate_ = v8::Isolate::New(create_params_);
        isolate_->Enter();

        // Constructing a v8::HandleScope automatically registers and attaches the scope to the
        // isolate. All local handles (v8::Local) created by napi_* calls are placed in this
        // active scope and kept alive until the scope is destroyed in TearDown().
        handle_scope_.emplace(isolate_);

        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context->Enter();
        env_ = dawn::napi_v8::CreateEnv(isolate_, context);
    }

    void TearDown() override {
        env_->GetContext()->Exit();
        dawn::napi_v8::DestroyEnv(env_);
        handle_scope_.reset();
        isolate_->Exit();
        isolate_->Dispose();
        allocator_.reset();
    }

    napi_env env_ = nullptr;

  private:
    static std::unique_ptr<v8::Platform> platform_;
    std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_;
    v8::Isolate::CreateParams create_params_;
    v8::Isolate* isolate_ = nullptr;
    std::optional<v8::HandleScope> handle_scope_;
};

std::unique_ptr<v8::Platform> NapiV8Test::platform_ = nullptr;

// ============================================================================
// Scopes Tests
// ============================================================================

TEST_F(NapiV8Test, HandleScopeLifecycle) {
    napi_handle_scope scope;
    ASSERT_EQ(napi_open_handle_scope(env_, &scope), napi_ok);

    napi_value val;
    ASSERT_EQ(napi_create_int32(env_, 42, &val), napi_ok);
    int32_t num = 0;
    ASSERT_EQ(napi_get_value_int32(env_, val, &num), napi_ok);
    EXPECT_EQ(num, 42);

    ASSERT_EQ(napi_close_handle_scope(env_, scope), napi_ok);
}

// Tests opening multiple nested handle scopes and closing them in LIFO order.
TEST_F(NapiV8Test, MultipleNestedHandleScopes) {
    napi_handle_scope scope1;
    ASSERT_EQ(napi_open_handle_scope(env_, &scope1), napi_ok);
    napi_value val1;
    ASSERT_EQ(napi_create_int32(env_, 100, &val1), napi_ok);

    napi_handle_scope scope2;
    ASSERT_EQ(napi_open_handle_scope(env_, &scope2), napi_ok);
    napi_value val2;
    ASSERT_EQ(napi_create_int32(env_, 200, &val2), napi_ok);

    napi_handle_scope scope3;
    ASSERT_EQ(napi_open_handle_scope(env_, &scope3), napi_ok);
    napi_value val3;
    ASSERT_EQ(napi_create_int32(env_, 300, &val3), napi_ok);

    // All values readable while all 3 scopes are open
    int32_t num = 0;
    ASSERT_EQ(napi_get_value_int32(env_, val1, &num), napi_ok);
    EXPECT_EQ(num, 100);
    ASSERT_EQ(napi_get_value_int32(env_, val2, &num), napi_ok);
    EXPECT_EQ(num, 200);
    ASSERT_EQ(napi_get_value_int32(env_, val3, &num), napi_ok);
    EXPECT_EQ(num, 300);

    // Close scope 3; outer scopes remain valid
    ASSERT_EQ(napi_close_handle_scope(env_, scope3), napi_ok);
    ASSERT_EQ(napi_get_value_int32(env_, val1, &num), napi_ok);
    EXPECT_EQ(num, 100);
    ASSERT_EQ(napi_get_value_int32(env_, val2, &num), napi_ok);
    EXPECT_EQ(num, 200);

    // Close scope 2; outermost scope remains valid
    ASSERT_EQ(napi_close_handle_scope(env_, scope2), napi_ok);
    ASSERT_EQ(napi_get_value_int32(env_, val1, &num), napi_ok);
    EXPECT_EQ(num, 100);

    ASSERT_EQ(napi_close_handle_scope(env_, scope1), napi_ok);
}

// Tests that attempting to close handle scopes out-of-order returns napi_handle_scope_mismatch.
TEST_F(NapiV8Test, HandleScopeMismatch) {
    napi_handle_scope outer_scope;
    ASSERT_EQ(napi_open_handle_scope(env_, &outer_scope), napi_ok);

    napi_handle_scope inner_scope;
    ASSERT_EQ(napi_open_handle_scope(env_, &inner_scope), napi_ok);

    // Attempting to close outer scope before inner scope should fail with mismatch
    EXPECT_EQ(napi_close_handle_scope(env_, outer_scope), napi_handle_scope_mismatch);

    const napi_extended_error_info* error_info = nullptr;
    ASSERT_EQ(napi_get_last_error_info(env_, &error_info), napi_ok);
    EXPECT_EQ(error_info->error_code, napi_handle_scope_mismatch);
    ASSERT_NE(error_info->error_message, nullptr);
    EXPECT_STREQ(error_info->error_message, "Handle scope closed out of order");

    // Proper LIFO closing succeeds
    ASSERT_EQ(napi_close_handle_scope(env_, inner_scope), napi_ok);
    ASSERT_EQ(napi_close_handle_scope(env_, outer_scope), napi_ok);
}

// ============================================================================
// Primitives Tests
// ============================================================================

TEST_F(NapiV8Test, SingletonsAndBooleans) {
    napi_value undefined_val;
    ASSERT_EQ(napi_get_undefined(env_, &undefined_val), napi_ok);
    napi_valuetype type;
    ASSERT_EQ(napi_typeof(env_, undefined_val, &type), napi_ok);
    EXPECT_EQ(type, napi_undefined);

    napi_value null_val;
    ASSERT_EQ(napi_get_null(env_, &null_val), napi_ok);
    ASSERT_EQ(napi_typeof(env_, null_val, &type), napi_ok);
    EXPECT_EQ(type, napi_null);

    napi_value global_val;
    ASSERT_EQ(napi_get_global(env_, &global_val), napi_ok);
    ASSERT_EQ(napi_typeof(env_, global_val, &type), napi_ok);
    EXPECT_EQ(type, napi_object);

    napi_value true_val;
    ASSERT_EQ(napi_get_boolean(env_, true, &true_val), napi_ok);
    bool bool_res = false;
    ASSERT_EQ(napi_get_value_bool(env_, true_val, &bool_res), napi_ok);
    EXPECT_TRUE(bool_res);

    napi_value false_val;
    ASSERT_EQ(napi_get_boolean(env_, false, &false_val), napi_ok);
    ASSERT_EQ(napi_get_value_bool(env_, false_val, &bool_res), napi_ok);
    EXPECT_FALSE(bool_res);
}

TEST_F(NapiV8Test, Numbers) {
    napi_value int_val;
    ASSERT_EQ(napi_create_int32(env_, -12345, &int_val), napi_ok);
    int32_t i32 = 0;
    ASSERT_EQ(napi_get_value_int32(env_, int_val, &i32), napi_ok);
    EXPECT_EQ(i32, -12345);

    napi_value uint_val;
    ASSERT_EQ(napi_create_uint32(env_, 987654u, &uint_val), napi_ok);
    uint32_t u32 = 0;
    ASSERT_EQ(napi_get_value_uint32(env_, uint_val, &u32), napi_ok);
    EXPECT_EQ(u32, 987654u);

    napi_value double_val;
    ASSERT_EQ(napi_create_double(env_, 3.1415926535, &double_val), napi_ok);
    double d = 0.0;
    ASSERT_EQ(napi_get_value_double(env_, double_val, &d), napi_ok);
    EXPECT_DOUBLE_EQ(d, 3.1415926535);

    napi_value int64_val;
    ASSERT_EQ(napi_create_int64(env_, 9007199254740991LL, &int64_val), napi_ok);
    int64_t i64 = 0;
    ASSERT_EQ(napi_get_value_int64(env_, int64_val, &i64), napi_ok);
    EXPECT_EQ(i64, 9007199254740991LL);
}

TEST_F(NapiV8Test, NumberBoundaryValues) {
    // 32-bit integer limits
    napi_value min_i32, max_i32;
    ASSERT_EQ(napi_create_int32(env_, INT32_MIN, &min_i32), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, INT32_MAX, &max_i32), napi_ok);
    int32_t i32_res = 0;
    ASSERT_EQ(napi_get_value_int32(env_, min_i32, &i32_res), napi_ok);
    EXPECT_EQ(i32_res, INT32_MIN);
    ASSERT_EQ(napi_get_value_int32(env_, max_i32, &i32_res), napi_ok);
    EXPECT_EQ(i32_res, INT32_MAX);

    // Unsigned 32-bit limit
    napi_value max_u32;
    ASSERT_EQ(napi_create_uint32(env_, UINT32_MAX, &max_u32), napi_ok);
    uint32_t u32_res = 0;
    ASSERT_EQ(napi_get_value_uint32(env_, max_u32, &u32_res), napi_ok);
    EXPECT_EQ(u32_res, UINT32_MAX);

    // Floating-point special values: Infinity, -Infinity, NaN
    napi_value inf_val, neg_inf_val, nan_val;
    ASSERT_EQ(napi_create_double(env_, std::numeric_limits<double>::infinity(), &inf_val), napi_ok);
    ASSERT_EQ(napi_create_double(env_, -std::numeric_limits<double>::infinity(), &neg_inf_val),
              napi_ok);
    ASSERT_EQ(napi_create_double(env_, std::numeric_limits<double>::quiet_NaN(), &nan_val),
              napi_ok);

    double d_res = 0.0;
    ASSERT_EQ(napi_get_value_double(env_, inf_val, &d_res), napi_ok);
    EXPECT_TRUE(std::isinf(d_res) && d_res > 0);
    ASSERT_EQ(napi_get_value_double(env_, neg_inf_val, &d_res), napi_ok);
    EXPECT_TRUE(std::isinf(d_res) && d_res < 0);
    ASSERT_EQ(napi_get_value_double(env_, nan_val, &d_res), napi_ok);
    EXPECT_TRUE(std::isnan(d_res));
}

TEST_F(NapiV8Test, Strings) {
    const char* test_str = "WebGPU CTS Runner on V8";
    napi_value str_val;
    ASSERT_EQ(napi_create_string_utf8(env_, test_str, NAPI_AUTO_LENGTH, &str_val), napi_ok);

    size_t length = 0;
    ASSERT_EQ(napi_get_value_string_utf8(env_, str_val, nullptr, 0, &length), napi_ok);
    EXPECT_EQ(length, strlen(test_str));

    std::vector<char> buf(length + 1);
    size_t written = 0;
    ASSERT_EQ(napi_get_value_string_utf8(env_, str_val, buf.data(), buf.size(), &written), napi_ok);
    EXPECT_EQ(written, length);
    EXPECT_STREQ(buf.data(), test_str);
}

// Tests string buffer sizing, truncation, and null-termination in napi_get_value_string_utf8.
TEST_F(NapiV8Test, StringUtf8TruncationAndNullTermination) {
    const char* test_str = "Hello, World!";
    napi_value str_val;
    ASSERT_EQ(napi_create_string_utf8(env_, test_str, NAPI_AUTO_LENGTH, &str_val), napi_ok);

    // Buffer smaller than string: capacity 6 should copy 5 chars + '\0'
    char small_buf[6] = {0};
    size_t written = 0;
    ASSERT_EQ(napi_get_value_string_utf8(env_, str_val, small_buf, sizeof(small_buf), &written),
              napi_ok);
    EXPECT_EQ(written, 5u);
    EXPECT_STREQ(small_buf, "Hello");
}

// Tests that when the destination buffer is larger than the string, the null-terminator
// is placed immediately after the copied string and remaining buffer memory is untouched.
TEST_F(NapiV8Test, StringUtf8OversizedBuffer) {
    const char* test_str = "hello";
    napi_value str_val;
    ASSERT_EQ(napi_create_string_utf8(env_, test_str, NAPI_AUTO_LENGTH, &str_val), napi_ok);

    std::array<char, 32> large_buf;
    large_buf.fill('x');
    size_t written = 0;
    ASSERT_EQ(
        napi_get_value_string_utf8(env_, str_val, large_buf.data(), large_buf.size(), &written),
        napi_ok);
    EXPECT_EQ(written, 5u);
    EXPECT_STREQ(large_buf.data(), "hello");
    EXPECT_EQ(large_buf[5], '\0');
    EXPECT_EQ(large_buf[6], 'x');
}

// Tests calling napi_get_value_string_utf8 with non-null buf and bufsize == 0.
// Verifies that nothing is written to buf (not even '\0') and result is set to 0.
TEST_F(NapiV8Test, StringUtf8ZeroBufferSize) {
    const char* test_str = "hello";
    napi_value str_val;
    ASSERT_EQ(napi_create_string_utf8(env_, test_str, NAPI_AUTO_LENGTH, &str_val), napi_ok);

    char sentinel_buf[4] = {'a', 'b', 'c', 'd'};
    size_t written = 999;
    ASSERT_EQ(napi_get_value_string_utf8(env_, str_val, sentinel_buf, 0, &written), napi_ok);
    EXPECT_EQ(written, 0u);
    EXPECT_EQ(sentinel_buf[0], 'a');
    EXPECT_EQ(sentinel_buf[1], 'b');
    EXPECT_EQ(sentinel_buf[2], 'c');
    EXPECT_EQ(sentinel_buf[3], 'd');

    // Also test with result == nullptr
    ASSERT_EQ(napi_get_value_string_utf8(env_, str_val, sentinel_buf, 0, nullptr), napi_ok);
    EXPECT_EQ(sentinel_buf[0], 'a');
}

// Tests empty string handling.
TEST_F(NapiV8Test, EmptyStrings) {
    napi_value empty_val;
    ASSERT_EQ(napi_create_string_utf8(env_, "", 0, &empty_val), napi_ok);

    size_t length = 99;
    ASSERT_EQ(napi_get_value_string_utf8(env_, empty_val, nullptr, 0, &length), napi_ok);
    EXPECT_EQ(length, 0u);

    char buf[4] = {'a', 'b', 'c', 'd'};
    size_t written = 99;
    ASSERT_EQ(napi_get_value_string_utf8(env_, empty_val, buf, sizeof(buf), &written), napi_ok);
    EXPECT_EQ(written, 0u);
    EXPECT_EQ(buf[0], '\0');
}

TEST_F(NapiV8Test, Symbols) {
    napi_value desc;
    ASSERT_EQ(napi_create_string_utf8(env_, "sym_desc", NAPI_AUTO_LENGTH, &desc), napi_ok);
    napi_value sym;
    ASSERT_EQ(napi_create_symbol(env_, desc, &sym), napi_ok);

    napi_valuetype type;
    ASSERT_EQ(napi_typeof(env_, sym, &type), napi_ok);
    EXPECT_EQ(type, napi_symbol);
}

TEST_F(NapiV8Test, CoercionsAndStrictEquals) {
    napi_value num_val;
    ASSERT_EQ(napi_create_int32(env_, 100, &num_val), napi_ok);

    napi_value str_coerced;
    ASSERT_EQ(napi_coerce_to_string(env_, num_val, &str_coerced), napi_ok);
    char buf[16] = {0};
    ASSERT_EQ(napi_get_value_string_utf8(env_, str_coerced, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "100");

    napi_value num2_val;
    ASSERT_EQ(napi_create_int32(env_, 100, &num2_val), napi_ok);
    bool equals = false;
    ASSERT_EQ(napi_strict_equals(env_, num_val, num2_val, &equals), napi_ok);
    EXPECT_TRUE(equals);

    napi_value diff_val;
    ASSERT_EQ(napi_create_int32(env_, 200, &diff_val), napi_ok);
    ASSERT_EQ(napi_strict_equals(env_, num_val, diff_val, &equals), napi_ok);
    EXPECT_FALSE(equals);
}

// Tests comprehensive napi_get_last_error_info lifecycle: initial state, argument validation,
// error code recording, reading idempotence, and clearing on subsequent successful calls.
TEST_F(NapiV8Test, GetLastErrorInfo) {
    const napi_extended_error_info* info = nullptr;

    // 1. Initial state: clean environment should report napi_ok
    ASSERT_EQ(napi_get_last_error_info(env_, &info), napi_ok);
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->error_code, napi_ok);
    EXPECT_EQ(info->error_message, nullptr);
    EXPECT_EQ(info->engine_error_code, 0u);
    EXPECT_EQ(info->engine_reserved, nullptr);

    // 2. Argument validation on napi_get_last_error_info itself
    EXPECT_EQ(napi_get_last_error_info(nullptr, &info), napi_invalid_arg);
    EXPECT_EQ(napi_get_last_error_info(env_, nullptr), napi_invalid_arg);

    // 3. Triggering an error records napi_invalid_arg and error message
    EXPECT_EQ(napi_create_int32(env_, 10, nullptr), napi_invalid_arg);
    ASSERT_EQ(napi_get_last_error_info(env_, &info), napi_ok);
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->error_code, napi_invalid_arg);
    ASSERT_NE(info->error_message, nullptr);
    EXPECT_STREQ(info->error_message, "Invalid argument: null pointer");

    // 4. Reading error info multiple times is idempotent (does not clear or alter error)
    const napi_extended_error_info* info2 = nullptr;
    ASSERT_EQ(napi_get_last_error_info(env_, &info2), napi_ok);
    EXPECT_EQ(info2->error_code, napi_invalid_arg);
    ASSERT_NE(info2->error_message, nullptr);
    EXPECT_STREQ(info2->error_message, "Invalid argument: null pointer");

    // 5. Triggering a different error code overwrites last_error_info and error message
    napi_value bool_val;
    ASSERT_EQ(napi_get_boolean(env_, true, &bool_val), napi_ok);
    char str_buf[16];
    EXPECT_EQ(napi_get_value_string_utf8(env_, bool_val, str_buf, sizeof(str_buf), nullptr),
              napi_string_expected);
    ASSERT_EQ(napi_get_last_error_info(env_, &info), napi_ok);
    EXPECT_EQ(info->error_code, napi_string_expected);
    ASSERT_NE(info->error_message, nullptr);
    EXPECT_STREQ(info->error_message, "A string was expected");

    // 6. Subsequent successful API call resets last_error_info back to napi_ok and clears message
    napi_value num_val;
    ASSERT_EQ(napi_create_int32(env_, 42, &num_val), napi_ok);
    ASSERT_EQ(napi_get_last_error_info(env_, &info), napi_ok);
    EXPECT_EQ(info->error_code, napi_ok);
    EXPECT_EQ(info->error_message, nullptr);
}

// Tests invalid argument validation and error propagation.
TEST_F(NapiV8Test, InvalidArgumentsAndErrorHandling) {
    // Passing null environment
    EXPECT_EQ(napi_create_int32(nullptr, 10, nullptr), napi_invalid_arg);

    // Passing null output pointer
    EXPECT_EQ(napi_create_int32(env_, 10, nullptr), napi_invalid_arg);

    // Type mismatch for extraction
    napi_value bool_val;
    ASSERT_EQ(napi_get_boolean(env_, true, &bool_val), napi_ok);
    char str_buf[16];
    EXPECT_EQ(napi_get_value_string_utf8(env_, bool_val, str_buf, sizeof(str_buf), nullptr),
              napi_string_expected);
}

}  // namespace
