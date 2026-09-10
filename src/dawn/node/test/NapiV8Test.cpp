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

// ============================================================================
// Object & Property Tests
// ============================================================================

TEST_F(NapiV8Test, ObjectsAndNamedProperties) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    napi_valuetype type;
    ASSERT_EQ(napi_typeof(env_, obj, &type), napi_ok);
    EXPECT_EQ(type, napi_object);

    // Initially property does not exist
    bool has_prop = true;
    ASSERT_EQ(napi_has_named_property(env_, obj, "foo", &has_prop), napi_ok);
    EXPECT_FALSE(has_prop);

    napi_value get_val;
    ASSERT_EQ(napi_get_named_property(env_, obj, "foo", &get_val), napi_ok);
    ASSERT_EQ(napi_typeof(env_, get_val, &type), napi_ok);
    EXPECT_EQ(type, napi_undefined);

    // Set named property
    napi_value str_val;
    ASSERT_EQ(napi_create_string_utf8(env_, "bar", NAPI_AUTO_LENGTH, &str_val), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj, "foo", str_val), napi_ok);

    // Check has property is true
    ASSERT_EQ(napi_has_named_property(env_, obj, "foo", &has_prop), napi_ok);
    EXPECT_TRUE(has_prop);

    // Get property value
    ASSERT_EQ(napi_get_named_property(env_, obj, "foo", &get_val), napi_ok);
    char buf[16];
    size_t written = 0;
    ASSERT_EQ(napi_get_value_string_utf8(env_, get_val, buf, sizeof(buf), &written), napi_ok);
    EXPECT_STREQ(buf, "bar");
    EXPECT_EQ(written, 3u);
}

TEST_F(NapiV8Test, ObjectsAndValueProperties) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    // 1. Initially, non-existent properties return false for has_property and undefined for
    // get_property
    napi_value non_existent_name;
    ASSERT_EQ(napi_create_string_utf8(env_, "missing", NAPI_AUTO_LENGTH, &non_existent_name),
              napi_ok);
    bool has_prop = true;
    ASSERT_EQ(napi_has_property(env_, obj, non_existent_name, &has_prop), napi_ok);
    EXPECT_FALSE(has_prop);

    napi_value missing_val;
    ASSERT_EQ(napi_get_property(env_, obj, non_existent_name, &missing_val), napi_ok);
    napi_valuetype val_type;
    ASSERT_EQ(napi_typeof(env_, missing_val, &val_type), napi_ok);
    EXPECT_EQ(val_type, napi_undefined);

    // 2. Non-existent symbol returns false
    napi_value missing_sym;
    ASSERT_EQ(napi_create_symbol(env_, nullptr, &missing_sym), napi_ok);
    ASSERT_EQ(napi_has_property(env_, obj, missing_sym, &has_prop), napi_ok);
    EXPECT_FALSE(has_prop);

    // 3. String key
    napi_value name_str, val_num;
    ASSERT_EQ(napi_create_string_utf8(env_, "key1", NAPI_AUTO_LENGTH, &name_str), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 1234, &val_num), napi_ok);
    ASSERT_EQ(napi_set_property(env_, obj, name_str, val_num), napi_ok);

    has_prop = false;
    ASSERT_EQ(napi_has_property(env_, obj, name_str, &has_prop), napi_ok);
    EXPECT_TRUE(has_prop);

    napi_value get_val;
    ASSERT_EQ(napi_get_property(env_, obj, name_str, &get_val), napi_ok);
    int32_t num = 0;
    ASSERT_EQ(napi_get_value_int32(env_, get_val, &num), napi_ok);
    EXPECT_EQ(num, 1234);

    // 4. Symbol key
    napi_value sym_name, sym_val;
    ASSERT_EQ(napi_create_symbol(env_, nullptr, &sym_name), napi_ok);
    ASSERT_EQ(napi_create_string_utf8(env_, "sym_value", NAPI_AUTO_LENGTH, &sym_val), napi_ok);
    ASSERT_EQ(napi_set_property(env_, obj, sym_name, sym_val), napi_ok);

    ASSERT_EQ(napi_has_property(env_, obj, sym_name, &has_prop), napi_ok);
    EXPECT_TRUE(has_prop);

    ASSERT_EQ(napi_get_property(env_, obj, sym_name, &get_val), napi_ok);
    char buf[16];
    ASSERT_EQ(napi_get_value_string_utf8(env_, get_val, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "sym_value");
}

TEST_F(NapiV8Test, ObjectPropertyNames) {
    // 1. Empty object returns array of length 0
    napi_value empty_obj;
    ASSERT_EQ(napi_create_object(env_, &empty_obj), napi_ok);
    napi_value empty_names;
    ASSERT_EQ(napi_get_property_names(env_, empty_obj, &empty_names), napi_ok);
    uint32_t empty_len = 999;
    ASSERT_EQ(napi_get_array_length(env_, empty_names, &empty_len), napi_ok);
    EXPECT_EQ(empty_len, 0u);

    // 2. Object with properties
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    napi_value val1, val2, val3;
    ASSERT_EQ(napi_create_int32(env_, 1, &val1), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 2, &val2), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 3, &val3), napi_ok);

    ASSERT_EQ(napi_set_named_property(env_, obj, "alpha", val1), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj, "beta", val2), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj, "gamma", val3), napi_ok);

    napi_value names;
    ASSERT_EQ(napi_get_property_names(env_, obj, &names), napi_ok);
    napi_valuetype type;
    ASSERT_EQ(napi_typeof(env_, names, &type), napi_ok);
    EXPECT_EQ(type, napi_object);

    uint32_t len = 0;
    ASSERT_EQ(napi_get_array_length(env_, names, &len), napi_ok);
    EXPECT_EQ(len, 3u);

    napi_value elem0, elem1, elem2;
    ASSERT_EQ(napi_get_element(env_, names, 0, &elem0), napi_ok);
    ASSERT_EQ(napi_get_element(env_, names, 1, &elem1), napi_ok);
    ASSERT_EQ(napi_get_element(env_, names, 2, &elem2), napi_ok);

    char buf[16];
    ASSERT_EQ(napi_get_value_string_utf8(env_, elem0, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "alpha");
    ASSERT_EQ(napi_get_value_string_utf8(env_, elem1, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "beta");
    ASSERT_EQ(napi_get_value_string_utf8(env_, elem2, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "gamma");
}

TEST_F(NapiV8Test, ObjectPrototypeProperties) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    // "toString" is on Object.prototype: has_property and has_named_property return true
    bool has_prop = false;
    ASSERT_EQ(napi_has_named_property(env_, obj, "toString", &has_prop), napi_ok);
    EXPECT_TRUE(has_prop);

    napi_value to_string_name;
    ASSERT_EQ(napi_create_string_utf8(env_, "toString", NAPI_AUTO_LENGTH, &to_string_name),
              napi_ok);
    ASSERT_EQ(napi_has_property(env_, obj, to_string_name, &has_prop), napi_ok);
    EXPECT_TRUE(has_prop);

    napi_value fn_val;
    ASSERT_EQ(napi_get_named_property(env_, obj, "toString", &fn_val), napi_ok);
    napi_valuetype type;
    ASSERT_EQ(napi_typeof(env_, fn_val, &type), napi_ok);
    EXPECT_EQ(type, napi_function);
}

// ============================================================================
// Array Tests
// ============================================================================

TEST_F(NapiV8Test, ArraysAndElements) {
    // 1. Create empty array
    napi_value arr;
    ASSERT_EQ(napi_create_array(env_, &arr), napi_ok);
    napi_valuetype type;
    ASSERT_EQ(napi_typeof(env_, arr, &type), napi_ok);
    EXPECT_EQ(type, napi_object);

    uint32_t len = 999;
    ASSERT_EQ(napi_get_array_length(env_, arr, &len), napi_ok);
    EXPECT_EQ(len, 0u);

    // 2. Set elements
    napi_value str1, str2;
    ASSERT_EQ(napi_create_string_utf8(env_, "first", NAPI_AUTO_LENGTH, &str1), napi_ok);
    ASSERT_EQ(napi_create_string_utf8(env_, "second", NAPI_AUTO_LENGTH, &str2), napi_ok);

    ASSERT_EQ(napi_set_element(env_, arr, 0, str1), napi_ok);
    ASSERT_EQ(napi_set_element(env_, arr, 1, str2), napi_ok);

    ASSERT_EQ(napi_get_array_length(env_, arr, &len), napi_ok);
    EXPECT_EQ(len, 2u);

    // 3. Get element
    napi_value elem_val;
    ASSERT_EQ(napi_get_element(env_, arr, 1, &elem_val), napi_ok);
    char buf[16];
    ASSERT_EQ(napi_get_value_string_utf8(env_, elem_val, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "second");

    // 4. Out-of-bounds get element returns undefined
    napi_value oob_val;
    ASSERT_EQ(napi_get_element(env_, arr, 99, &oob_val), napi_ok);
    ASSERT_EQ(napi_typeof(env_, oob_val, &type), napi_ok);
    EXPECT_EQ(type, napi_undefined);

    // 5. Sparse array: setting element at index 5 grows length to 6
    napi_value str_sparse;
    ASSERT_EQ(napi_create_string_utf8(env_, "sparse", NAPI_AUTO_LENGTH, &str_sparse), napi_ok);
    ASSERT_EQ(napi_set_element(env_, arr, 5, str_sparse), napi_ok);
    ASSERT_EQ(napi_get_array_length(env_, arr, &len), napi_ok);
    EXPECT_EQ(len, 6u);

    // Hole index 3 returns undefined
    napi_value hole_val;
    ASSERT_EQ(napi_get_element(env_, arr, 3, &hole_val), napi_ok);
    ASSERT_EQ(napi_typeof(env_, hole_val, &type), napi_ok);
    EXPECT_EQ(type, napi_undefined);

    // 6. Create array with length
    napi_value arr2;
    ASSERT_EQ(napi_create_array_with_length(env_, 10, &arr2), napi_ok);
    ASSERT_EQ(napi_get_array_length(env_, arr2, &len), napi_ok);
    EXPECT_EQ(len, 10u);
}

TEST_F(NapiV8Test, IndexedPropertiesOnPlainObject) {
    // JavaScript objects can have indexed elements
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    napi_value val;
    ASSERT_EQ(napi_create_string_utf8(env_, "indexed_val", NAPI_AUTO_LENGTH, &val), napi_ok);
    ASSERT_EQ(napi_set_element(env_, obj, 0, val), napi_ok);

    napi_value get_val;
    ASSERT_EQ(napi_get_element(env_, obj, 0, &get_val), napi_ok);
    char buf[32];
    ASSERT_EQ(napi_get_value_string_utf8(env_, get_val, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "indexed_val");

    bool has_named = false;
    ASSERT_EQ(napi_has_named_property(env_, obj, "0", &has_named), napi_ok);
    EXPECT_TRUE(has_named);
}

TEST_F(NapiV8Test, ObjectAndArrayInvalidArgs) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    napi_value arr;
    ASSERT_EQ(napi_create_array(env_, &arr), napi_ok);
    napi_value name;
    ASSERT_EQ(napi_create_string_utf8(env_, "k", NAPI_AUTO_LENGTH, &name), napi_ok);
    napi_value val;
    ASSERT_EQ(napi_create_int32(env_, 1, &val), napi_ok);

    // 1. napi_create_object / napi_create_array
    EXPECT_EQ(napi_create_object(nullptr, &obj), napi_invalid_arg);
    EXPECT_EQ(napi_create_object(env_, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_create_array(nullptr, &arr), napi_invalid_arg);
    EXPECT_EQ(napi_create_array(env_, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_create_array_with_length(nullptr, 5, &arr), napi_invalid_arg);
    EXPECT_EQ(napi_create_array_with_length(env_, 5, nullptr), napi_invalid_arg);

    // 2. napi_get_property_names
    napi_value names;
    EXPECT_EQ(napi_get_property_names(nullptr, obj, &names), napi_invalid_arg);
    EXPECT_EQ(napi_get_property_names(env_, nullptr, &names), napi_invalid_arg);
    EXPECT_EQ(napi_get_property_names(env_, obj, nullptr), napi_invalid_arg);

    // 3. napi_set_property / napi_get_property / napi_has_property
    napi_value get_res;
    bool has_res;
    EXPECT_EQ(napi_set_property(nullptr, obj, name, val), napi_invalid_arg);
    EXPECT_EQ(napi_set_property(env_, nullptr, name, val), napi_invalid_arg);
    EXPECT_EQ(napi_set_property(env_, obj, nullptr, val), napi_invalid_arg);
    EXPECT_EQ(napi_set_property(env_, obj, name, nullptr), napi_invalid_arg);

    EXPECT_EQ(napi_get_property(nullptr, obj, name, &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_property(env_, nullptr, name, &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_property(env_, obj, nullptr, &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_property(env_, obj, name, nullptr), napi_invalid_arg);

    EXPECT_EQ(napi_has_property(nullptr, obj, name, &has_res), napi_invalid_arg);
    EXPECT_EQ(napi_has_property(env_, nullptr, name, &has_res), napi_invalid_arg);
    EXPECT_EQ(napi_has_property(env_, obj, nullptr, &has_res), napi_invalid_arg);
    EXPECT_EQ(napi_has_property(env_, obj, name, nullptr), napi_invalid_arg);

    // 4. napi_set_named_property / napi_get_named_property / napi_has_named_property
    EXPECT_EQ(napi_set_named_property(nullptr, obj, "k", val), napi_invalid_arg);
    EXPECT_EQ(napi_set_named_property(env_, nullptr, "k", val), napi_invalid_arg);
    EXPECT_EQ(napi_set_named_property(env_, obj, nullptr, val), napi_invalid_arg);
    EXPECT_EQ(napi_set_named_property(env_, obj, "k", nullptr), napi_invalid_arg);

    EXPECT_EQ(napi_get_named_property(nullptr, obj, "k", &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_named_property(env_, nullptr, "k", &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_named_property(env_, obj, nullptr, &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_named_property(env_, obj, "k", nullptr), napi_invalid_arg);

    EXPECT_EQ(napi_has_named_property(nullptr, obj, "k", &has_res), napi_invalid_arg);
    EXPECT_EQ(napi_has_named_property(env_, nullptr, "k", &has_res), napi_invalid_arg);
    EXPECT_EQ(napi_has_named_property(env_, obj, nullptr, &has_res), napi_invalid_arg);
    EXPECT_EQ(napi_has_named_property(env_, obj, "k", nullptr), napi_invalid_arg);

    // 5. napi_get_element / napi_set_element
    EXPECT_EQ(napi_get_element(nullptr, arr, 0, &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_element(env_, nullptr, 0, &get_res), napi_invalid_arg);
    EXPECT_EQ(napi_get_element(env_, arr, 0, nullptr), napi_invalid_arg);

    EXPECT_EQ(napi_set_element(nullptr, arr, 0, val), napi_invalid_arg);
    EXPECT_EQ(napi_set_element(env_, nullptr, 0, val), napi_invalid_arg);
    EXPECT_EQ(napi_set_element(env_, arr, 0, nullptr), napi_invalid_arg);

    // 6. Calling array_length on non-array
    uint32_t len = 0;
    EXPECT_EQ(napi_get_array_length(nullptr, arr, &len), napi_invalid_arg);
    EXPECT_EQ(napi_get_array_length(env_, nullptr, &len), napi_invalid_arg);
    EXPECT_EQ(napi_get_array_length(env_, arr, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_get_array_length(env_, obj, &len), napi_array_expected);

    const napi_extended_error_info* info = nullptr;
    ASSERT_EQ(napi_get_last_error_info(env_, &info), napi_ok);
    EXPECT_EQ(info->error_code, napi_array_expected);
    ASSERT_NE(info->error_message, nullptr);
    EXPECT_STREQ(info->error_message, "An array was expected");
}

// ============================================================================
// Stage 4: Functions, Classes & Properties Tests
// ============================================================================

// 1. Function Creation & Metadata
TEST_F(NapiV8Test, FunctionCreationAndTypeOf) {
    auto noop_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "testFn", NAPI_AUTO_LENGTH, noop_cb, nullptr, &fn),
              napi_ok);
    napi_valuetype fn_type;
    ASSERT_EQ(napi_typeof(env_, fn, &fn_type), napi_ok);
    EXPECT_EQ(fn_type, napi_function);
}

TEST_F(NapiV8Test, FunctionNameAutoLength) {
    auto noop_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    napi_value fn;
    ASSERT_EQ(
        napi_create_function(env_, "myAwesomeFunction", NAPI_AUTO_LENGTH, noop_cb, nullptr, &fn),
        napi_ok);
    napi_value name_val;
    ASSERT_EQ(napi_get_named_property(env_, fn, "name", &name_val), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, name_val, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "myAwesomeFunction");
}

TEST_F(NapiV8Test, FunctionNameExplicitLength) {
    auto noop_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "longFunctionName", 4, noop_cb, nullptr, &fn), napi_ok);
    napi_value name_val;
    ASSERT_EQ(napi_get_named_property(env_, fn, "name", &name_val), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, name_val, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "long");
}

TEST_F(NapiV8Test, FunctionAnonymous) {
    auto noop_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, nullptr, NAPI_AUTO_LENGTH, noop_cb, nullptr, &fn),
              napi_ok);
    napi_value name_val;
    ASSERT_EQ(napi_get_named_property(env_, fn, "name", &name_val), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, name_val, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "");
}

// 2. Invocation & Argument Counts
TEST_F(NapiV8Test, FunctionCallZeroArgs) {
    auto zero_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 5;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        EXPECT_EQ(argc, 0u);
        napi_value res;
        napi_create_int32(env, 42, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "zeroArgs", NAPI_AUTO_LENGTH, zero_cb, nullptr, &fn),
              napi_ok);
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 0, nullptr, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 42);
}

TEST_F(NapiV8Test, FunctionCallSingleArg) {
    auto double_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_get_cb_info(env, info, &argc, &arg, nullptr, nullptr);
        EXPECT_EQ(argc, 1u);
        int32_t x = 0;
        napi_get_value_int32(env, arg, &x);
        napi_value res;
        napi_create_int32(env, x * 2, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "doubleFn", NAPI_AUTO_LENGTH, double_cb, nullptr, &fn),
              napi_ok);
    napi_value arg;
    ASSERT_EQ(napi_create_int32(env_, 21, &arg), napi_ok);
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 1, &arg, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 42);
}

TEST_F(NapiV8Test, FunctionCallTwoArgs) {
    auto add_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 2;
        napi_value args[2];
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 2u);
        int32_t a = 0, b = 0;
        napi_get_value_int32(env, args[0], &a);
        napi_get_value_int32(env, args[1], &b);
        napi_value res;
        napi_create_int32(env, a + b, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "add", NAPI_AUTO_LENGTH, add_cb, nullptr, &fn), napi_ok);
    napi_value a, b;
    ASSERT_EQ(napi_create_int32(env_, 15, &a), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 25, &b), napi_ok);
    napi_value argv[2] = {a, b};
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 2, argv, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 40);
}

TEST_F(NapiV8Test, FunctionCallStringArgs) {
    auto concat_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 2;
        napi_value args[2] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 2u);

        char buf1[32];
        char buf2[32];
        napi_get_value_string_utf8(env, args[0], buf1, sizeof(buf1), nullptr);
        napi_get_value_string_utf8(env, args[1], buf2, sizeof(buf2), nullptr);

        std::string combined = std::string(buf1) + buf2;
        napi_value res;
        napi_create_string_utf8(env, combined.c_str(), combined.length(), &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "concatFn", NAPI_AUTO_LENGTH, concat_cb, nullptr, &fn),
              napi_ok);

    napi_value s1, s2;
    ASSERT_EQ(napi_create_string_utf8(env_, "Hello ", NAPI_AUTO_LENGTH, &s1), napi_ok);
    ASSERT_EQ(napi_create_string_utf8(env_, "World!", NAPI_AUTO_LENGTH, &s2), napi_ok);
    napi_value argv[2] = {s1, s2};

    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 2, argv, &call_res), napi_ok);
    char out_buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, call_res, out_buf, sizeof(out_buf), nullptr),
              napi_ok);
    EXPECT_STREQ(out_buf, "Hello World!");
}

TEST_F(NapiV8Test, FunctionCallBooleanAndDoubleArgs) {
    auto calc_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 2;
        napi_value args[2] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 2u);

        bool flag = false;
        double factor = 0.0;
        napi_get_value_bool(env, args[0], &flag);
        napi_get_value_double(env, args[1], &factor);

        double result_val = flag ? (factor * 2.5) : (factor * 0.5);
        napi_value res;
        napi_create_double(env, result_val, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "calcFn", NAPI_AUTO_LENGTH, calc_cb, nullptr, &fn),
              napi_ok);

    napi_value factor_val;
    ASSERT_EQ(napi_create_double(env_, 4.0, &factor_val), napi_ok);

    // Call 1: flag = true -> 4.0 * 2.5 = 10.0
    napi_value true_val;
    ASSERT_EQ(napi_get_boolean(env_, true, &true_val), napi_ok);
    napi_value argv_true[2] = {true_val, factor_val};
    napi_value res_true;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 2, argv_true, &res_true), napi_ok);
    double num_true = 0.0;
    ASSERT_EQ(napi_get_value_double(env_, res_true, &num_true), napi_ok);
    EXPECT_DOUBLE_EQ(num_true, 10.0);

    // Call 2: flag = false -> 4.0 * 0.5 = 2.0
    napi_value false_val;
    ASSERT_EQ(napi_get_boolean(env_, false, &false_val), napi_ok);
    napi_value argv_false[2] = {false_val, factor_val};
    napi_value res_false;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 2, argv_false, &res_false), napi_ok);
    double num_false = 0.0;
    ASSERT_EQ(napi_get_value_double(env_, res_false, &num_false), napi_ok);
    EXPECT_DOUBLE_EQ(num_false, 2.0);
}

TEST_F(NapiV8Test, FunctionCallObjectArg) {
    auto obj_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_get_cb_info(env, info, &argc, &arg, nullptr, nullptr);
        EXPECT_EQ(argc, 1u);

        napi_valuetype arg_type;
        napi_typeof(env, arg, &arg_type);
        EXPECT_EQ(arg_type, napi_object);

        napi_value x_val, y_val;
        napi_get_named_property(env, arg, "x", &x_val);
        napi_get_named_property(env, arg, "y", &y_val);

        int32_t x = 0, y = 0;
        napi_get_value_int32(env, x_val, &x);
        napi_get_value_int32(env, y_val, &y);

        napi_value res;
        napi_create_int32(env, x + y, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "objFn", NAPI_AUTO_LENGTH, obj_cb, nullptr, &fn), napi_ok);

    napi_value input_obj;
    ASSERT_EQ(napi_create_object(env_, &input_obj), napi_ok);
    napi_value x, y;
    ASSERT_EQ(napi_create_int32(env_, 100, &x), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 200, &y), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, input_obj, "x", x), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, input_obj, "y", y), napi_ok);

    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 1, &input_obj, &call_res), napi_ok);
    int32_t sum = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &sum), napi_ok);
    EXPECT_EQ(sum, 300);
}

TEST_F(NapiV8Test, FunctionCallArrayArg) {
    auto array_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_get_cb_info(env, info, &argc, &arg, nullptr, nullptr);
        EXPECT_EQ(argc, 1u);

        uint32_t len = 0;
        napi_get_array_length(env, arg, &len);
        EXPECT_EQ(len, 3u);

        int32_t total = 0;
        for (uint32_t i = 0; i < len; ++i) {
            napi_value elem;
            napi_get_element(env, arg, i, &elem);
            int32_t v = 0;
            napi_get_value_int32(env, elem, &v);
            total += v;
        }

        napi_value res;
        napi_create_int32(env, total, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "arrayFn", NAPI_AUTO_LENGTH, array_cb, nullptr, &fn),
              napi_ok);

    napi_value arr;
    ASSERT_EQ(napi_create_array(env_, &arr), napi_ok);
    napi_value e0, e1, e2;
    ASSERT_EQ(napi_create_int32(env_, 10, &e0), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 20, &e1), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 30, &e2), napi_ok);
    ASSERT_EQ(napi_set_element(env_, arr, 0, e0), napi_ok);
    ASSERT_EQ(napi_set_element(env_, arr, 1, e1), napi_ok);
    ASSERT_EQ(napi_set_element(env_, arr, 2, e2), napi_ok);

    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 1, &arr, &call_res), napi_ok);
    int32_t sum = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &sum), napi_ok);
    EXPECT_EQ(sum, 60);
}

TEST_F(NapiV8Test, FunctionCallNullAndUndefinedArgs) {
    auto null_undef_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 2;
        napi_value args[2] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 2u);

        napi_valuetype t0, t1;
        napi_typeof(env, args[0], &t0);
        napi_typeof(env, args[1], &t1);
        EXPECT_EQ(t0, napi_null);
        EXPECT_EQ(t1, napi_undefined);

        napi_value res;
        napi_get_boolean(env, true, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(
        napi_create_function(env_, "nullUndefFn", NAPI_AUTO_LENGTH, null_undef_cb, nullptr, &fn),
        napi_ok);

    napi_value null_val, undef_val;
    ASSERT_EQ(napi_get_null(env_, &null_val), napi_ok);
    ASSERT_EQ(napi_get_undefined(env_, &undef_val), napi_ok);
    napi_value argv[2] = {null_val, undef_val};

    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 2, argv, &call_res), napi_ok);
    bool b_val = false;
    ASSERT_EQ(napi_get_value_bool(env_, call_res, &b_val), napi_ok);
    EXPECT_TRUE(b_val);
}

TEST_F(NapiV8Test, FunctionCallMixedArgs) {
    auto mixed_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 4;
        napi_value args[4] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 4u);

        napi_valuetype t0, t1, t2, t3;
        napi_typeof(env, args[0], &t0);
        napi_typeof(env, args[1], &t1);
        napi_typeof(env, args[2], &t2);
        napi_typeof(env, args[3], &t3);
        EXPECT_EQ(t0, napi_string);
        EXPECT_EQ(t1, napi_number);
        EXPECT_EQ(t2, napi_boolean);
        EXPECT_EQ(t3, napi_object);

        char prefix[32];
        napi_get_value_string_utf8(env, args[0], prefix, sizeof(prefix), nullptr);
        int32_t count = 0;
        napi_get_value_int32(env, args[1], &count);
        bool enabled = false;
        napi_get_value_bool(env, args[2], &enabled);

        napi_value tag_prop;
        napi_get_named_property(env, args[3], "tag", &tag_prop);
        char tag_str[32];
        napi_get_value_string_utf8(env, tag_prop, tag_str, sizeof(tag_str), nullptr);

        std::string result_str = std::string(prefix) + ":" + std::to_string(count) + ":" +
                                 (enabled ? "on" : "off") + ":" + tag_str;
        napi_value res;
        napi_create_string_utf8(env, result_str.c_str(), result_str.length(), &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "mixedFn", NAPI_AUTO_LENGTH, mixed_cb, nullptr, &fn),
              napi_ok);

    napi_value str_val, num_val, bool_val, obj_val, tag_val;
    ASSERT_EQ(napi_create_string_utf8(env_, "config", NAPI_AUTO_LENGTH, &str_val), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 7, &num_val), napi_ok);
    ASSERT_EQ(napi_get_boolean(env_, true, &bool_val), napi_ok);
    ASSERT_EQ(napi_create_object(env_, &obj_val), napi_ok);
    ASSERT_EQ(napi_create_string_utf8(env_, "v1", NAPI_AUTO_LENGTH, &tag_val), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj_val, "tag", tag_val), napi_ok);

    napi_value argv[4] = {str_val, num_val, bool_val, obj_val};
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 4, argv, &call_res), napi_ok);

    char out_buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, call_res, out_buf, sizeof(out_buf), nullptr),
              napi_ok);
    EXPECT_STREQ(out_buf, "config:7:on:v1");
}

TEST_F(NapiV8Test, FunctionCallManyArgs) {
    auto sum_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 5;
        napi_value args[5] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 5u);
        int32_t v0 = 0, v1 = 0, v2 = 0, v3 = 0, v4 = 0;
        napi_get_value_int32(env, args[0], &v0);
        napi_get_value_int32(env, args[1], &v1);
        napi_get_value_int32(env, args[2], &v2);
        napi_get_value_int32(env, args[3], &v3);
        napi_get_value_int32(env, args[4], &v4);
        napi_value res;
        napi_create_int32(env, v0 + v1 + v2 + v3 + v4, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "sumFive", NAPI_AUTO_LENGTH, sum_cb, nullptr, &fn),
              napi_ok);
    napi_value a, b, c, d, e;
    ASSERT_EQ(napi_create_int32(env_, 10, &a), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 20, &b), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 30, &c), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 40, &d), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 50, &e), napi_ok);
    napi_value argv[5] = {a, b, c, d, e};
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 5, argv, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 150);
}

TEST_F(NapiV8Test, FunctionCallFewerArgsThanExpected) {
    auto cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 3;
        napi_value args[3] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 1u);
        int32_t val = 0;
        napi_get_value_int32(env, args[0], &val);

        // Missing arguments are populated with undefined
        napi_valuetype t1, t2;
        napi_typeof(env, args[1], &t1);
        EXPECT_EQ(t1, napi_undefined);
        napi_typeof(env, args[2], &t2);
        EXPECT_EQ(t2, napi_undefined);

        napi_value res;
        napi_create_int32(env, val * 10, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "fewerArgs", NAPI_AUTO_LENGTH, cb, nullptr, &fn), napi_ok);
    napi_value arg;
    ASSERT_EQ(napi_create_int32(env_, 7, &arg), napi_ok);
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 1, &arg, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 70);
}

TEST_F(NapiV8Test, FunctionCallMoreArgsThanExpected) {
    auto cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 2;
        napi_value args[2] = {};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        EXPECT_EQ(argc, 4u);
        int32_t a = 0, b = 0;
        napi_get_value_int32(env, args[0], &a);
        napi_get_value_int32(env, args[1], &b);
        napi_value res;
        napi_create_int32(env, a + b, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "moreArgs", NAPI_AUTO_LENGTH, cb, nullptr, &fn), napi_ok);
    napi_value a, b, c, d;
    ASSERT_EQ(napi_create_int32(env_, 100, &a), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 200, &b), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 300, &c), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 400, &d), napi_ok);
    napi_value argv[4] = {a, b, c, d};
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 4, argv, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 300);
}

TEST_F(NapiV8Test, FunctionCallbackUserData) {
    int32_t custom_payload = 777;
    auto cb = [](napi_env env, napi_callback_info info) -> napi_value {
        void* data = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
        EXPECT_NE(data, nullptr);
        int32_t payload_val = *static_cast<int32_t*>(data);
        napi_value res;
        napi_create_int32(env, payload_val + 1, &res);
        return res;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "dataFn", NAPI_AUTO_LENGTH, cb, &custom_payload, &fn),
              napi_ok);
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, nullptr, fn, 0, nullptr, &call_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, call_res, &val), napi_ok);
    EXPECT_EQ(val, 778);
}

TEST_F(NapiV8Test, FunctionCallbackThisArg) {
    napi_value receiver;
    ASSERT_EQ(napi_create_object(env_, &receiver), napi_ok);

    // Verify property does not exist on receiver prior to invocation
    bool has_prop = true;
    ASSERT_EQ(napi_has_named_property(env_, receiver, "modifiedByCallback", &has_prop), napi_ok);
    EXPECT_FALSE(has_prop);

    auto cb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value this_arg;
        napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
        napi_value v;
        napi_create_int32(env, 999, &v);
        napi_set_named_property(env, this_arg, "modifiedByCallback", v);
        return nullptr;
    };
    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "thisFn", NAPI_AUTO_LENGTH, cb, nullptr, &fn), napi_ok);
    napi_value call_res;
    ASSERT_EQ(napi_call_function(env_, receiver, fn, 0, nullptr, &call_res), napi_ok);

    // Verify property now exists on receiver with the value set by callback
    ASSERT_EQ(napi_has_named_property(env_, receiver, "modifiedByCallback", &has_prop), napi_ok);
    EXPECT_TRUE(has_prop);

    napi_value prop_val;
    ASSERT_EQ(napi_get_named_property(env_, receiver, "modifiedByCallback", &prop_val), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, prop_val, &val), napi_ok);
    EXPECT_EQ(val, 999);
}

// 3. Classes & Constructors
TEST_F(NapiV8Test, ClassConstructor) {
    auto ctor_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_value this_arg;
        napi_get_cb_info(env, info, &argc, &arg, &this_arg, nullptr);
        int32_t init_val = 0;
        if (argc > 0) {
            napi_get_value_int32(env, arg, &init_val);
        }
        napi_value v;
        napi_create_int32(env, init_val, &v);
        napi_set_named_property(env, this_arg, "_val", v);
        return nullptr;
    };

    napi_value ctor;
    ASSERT_EQ(
        napi_define_class(env_, "MyClass", NAPI_AUTO_LENGTH, ctor_cb, nullptr, 0, nullptr, &ctor),
        napi_ok);

    napi_value arg;
    ASSERT_EQ(napi_create_int32(env_, 42, &arg), napi_ok);
    napi_value inst;
    ASSERT_EQ(napi_new_instance(env_, ctor, 1, &arg, &inst), napi_ok);

    napi_valuetype inst_type;
    ASSERT_EQ(napi_typeof(env_, inst, &inst_type), napi_ok);
    EXPECT_EQ(inst_type, napi_object);

    napi_value val_prop;
    ASSERT_EQ(napi_get_named_property(env_, inst, "_val", &val_prop), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, val_prop, &val), napi_ok);
    EXPECT_EQ(val, 42);
}

TEST_F(NapiV8Test, ClassInstanceMethods) {
    auto ctor_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value this_arg;
        napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
        napi_value v;
        napi_create_int32(env, 10, &v);
        napi_set_named_property(env, this_arg, "_base", v);
        return nullptr;
    };

    auto multiply_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_value this_arg;
        napi_get_cb_info(env, info, &argc, &arg, &this_arg, nullptr);
        int32_t factor = 1;
        if (argc > 0) {
            napi_get_value_int32(env, arg, &factor);
        }
        napi_value base_prop;
        napi_get_named_property(env, this_arg, "_base", &base_prop);
        int32_t base = 0;
        napi_get_value_int32(env, base_prop, &base);
        napi_value res;
        napi_create_int32(env, base * factor, &res);
        return res;
    };

    napi_property_descriptor props[] = {
        {"multiply", nullptr, multiply_cb, nullptr, nullptr, nullptr, napi_default, nullptr},
    };

    napi_value ctor;
    ASSERT_EQ(
        napi_define_class(env_, "Multiplier", NAPI_AUTO_LENGTH, ctor_cb, nullptr, 1, props, &ctor),
        napi_ok);

    napi_value inst;
    ASSERT_EQ(napi_new_instance(env_, ctor, 0, nullptr, &inst), napi_ok);

    napi_value method_fn;
    ASSERT_EQ(napi_get_named_property(env_, inst, "multiply", &method_fn), napi_ok);

    napi_value factor_arg;
    ASSERT_EQ(napi_create_int32(env_, 5, &factor_arg), napi_ok);
    napi_value method_res;
    ASSERT_EQ(napi_call_function(env_, inst, method_fn, 1, &factor_arg, &method_res), napi_ok);
    int32_t res_val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, method_res, &res_val), napi_ok);
    EXPECT_EQ(res_val, 50);
}

TEST_F(NapiV8Test, ClassGettersAndSetters) {
    auto ctor_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    auto getter_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value this_arg;
        napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
        napi_value v;
        napi_get_named_property(env, this_arg, "_hidden", &v);
        return v;
    };
    auto setter_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_value this_arg;
        napi_get_cb_info(env, info, &argc, &arg, &this_arg, nullptr);
        napi_set_named_property(env, this_arg, "_hidden", arg);
        return nullptr;
    };

    napi_property_descriptor props[] = {
        {"val", nullptr, nullptr, getter_cb, setter_cb, nullptr, napi_default, nullptr},
    };

    napi_value ctor;
    ASSERT_EQ(napi_define_class(env_, "AccessorClass", NAPI_AUTO_LENGTH, ctor_cb, nullptr, 1, props,
                                &ctor),
              napi_ok);

    napi_value inst;
    ASSERT_EQ(napi_new_instance(env_, ctor, 0, nullptr, &inst), napi_ok);

    // Set val = 123
    napi_value new_val;
    ASSERT_EQ(napi_create_int32(env_, 123, &new_val), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, inst, "val", new_val), napi_ok);

    // Read val via getter
    napi_value read_val;
    ASSERT_EQ(napi_get_named_property(env_, inst, "val", &read_val), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, read_val, &val), napi_ok);
    EXPECT_EQ(val, 123);
}

TEST_F(NapiV8Test, ClassStaticProperties) {
    auto ctor_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    auto static_cb = [](napi_env env, napi_callback_info) -> napi_value {
        napi_value res;
        napi_create_int32(env, 888, &res);
        return res;
    };

    napi_property_descriptor props[] = {
        {"staticMethod", nullptr, static_cb, nullptr, nullptr, nullptr, napi_static, nullptr},
    };

    napi_value ctor;
    ASSERT_EQ(
        napi_define_class(env_, "StaticClass", NAPI_AUTO_LENGTH, ctor_cb, nullptr, 1, props, &ctor),
        napi_ok);

    // Static method is available on constructor function
    napi_value static_fn;
    ASSERT_EQ(napi_get_named_property(env_, ctor, "staticMethod", &static_fn), napi_ok);
    napi_value static_res;
    ASSERT_EQ(napi_call_function(env_, ctor, static_fn, 0, nullptr, &static_res), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, static_res, &val), napi_ok);
    EXPECT_EQ(val, 888);

    // Static method is NOT on instances
    napi_value inst;
    ASSERT_EQ(napi_new_instance(env_, ctor, 0, nullptr, &inst), napi_ok);
    bool has_on_inst = true;
    ASSERT_EQ(napi_has_named_property(env_, inst, "staticMethod", &has_on_inst), napi_ok);
    EXPECT_FALSE(has_on_inst);
}

TEST_F(NapiV8Test, ClassStaticAndPrototypeValues) {
    auto ctor_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };

    napi_value static_val;
    ASSERT_EQ(napi_create_int32(env_, 777, &static_val), napi_ok);

    napi_value proto_val;
    ASSERT_EQ(napi_create_string_utf8(env_, "constant_tag", NAPI_AUTO_LENGTH, &proto_val), napi_ok);

    napi_property_descriptor props[] = {
        {"STATIC_CONST", nullptr, nullptr, nullptr, nullptr, static_val,
         static_cast<napi_property_attributes>(napi_static | napi_enumerable), nullptr},
        {"PROTO_CONST", nullptr, nullptr, nullptr, nullptr, proto_val, napi_enumerable, nullptr},
    };

    napi_value ctor;
    ASSERT_EQ(
        napi_define_class(env_, "ConstClass", NAPI_AUTO_LENGTH, ctor_cb, nullptr, 2, props, &ctor),
        napi_ok);

    // Static value is on constructor
    napi_value read_static;
    ASSERT_EQ(napi_get_named_property(env_, ctor, "STATIC_CONST", &read_static), napi_ok);
    int32_t s_val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, read_static, &s_val), napi_ok);
    EXPECT_EQ(s_val, 777);

    // Static value is NOT on instance
    napi_value inst;
    ASSERT_EQ(napi_new_instance(env_, ctor, 0, nullptr, &inst), napi_ok);
    bool has_static_on_inst = true;
    ASSERT_EQ(napi_has_named_property(env_, inst, "STATIC_CONST", &has_static_on_inst), napi_ok);
    EXPECT_FALSE(has_static_on_inst);

    // Prototype constant value is inherited by instance
    napi_value read_proto;
    ASSERT_EQ(napi_get_named_property(env_, inst, "PROTO_CONST", &read_proto), napi_ok);
    char tag_buf[32];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_proto, tag_buf, sizeof(tag_buf), nullptr),
              napi_ok);
    EXPECT_STREQ(tag_buf, "constant_tag");
}

TEST_F(NapiV8Test, ClassEmptyProperties) {
    auto ctor_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    napi_value ctor;
    ASSERT_EQ(napi_define_class(env_, "EmptyClass", NAPI_AUTO_LENGTH, ctor_cb, nullptr, 0, nullptr,
                                &ctor),
              napi_ok);
    napi_value inst;
    ASSERT_EQ(napi_new_instance(env_, ctor, 0, nullptr, &inst), napi_ok);
    napi_valuetype inst_type;
    ASSERT_EQ(napi_typeof(env_, inst, &inst_type), napi_ok);
    EXPECT_EQ(inst_type, napi_object);
}

// 4. Object Properties
TEST_F(NapiV8Test, DefineObjectMethods) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    auto method_cb = [](napi_env env, napi_callback_info) -> napi_value {
        napi_value res;
        napi_create_string_utf8(env, "hello_method", NAPI_AUTO_LENGTH, &res);
        return res;
    };

    napi_property_descriptor descs[] = {
        {"myMethod", nullptr, method_cb, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    ASSERT_EQ(napi_define_properties(env_, obj, 1, descs), napi_ok);

    napi_value fn;
    ASSERT_EQ(napi_get_named_property(env_, obj, "myMethod", &fn), napi_ok);
    napi_value ret;
    ASSERT_EQ(napi_call_function(env_, obj, fn, 0, nullptr, &ret), napi_ok);
    char buf[32];
    ASSERT_EQ(napi_get_value_string_utf8(env_, ret, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "hello_method");
}

TEST_F(NapiV8Test, DefineObjectValues) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    napi_value num_val;
    ASSERT_EQ(napi_create_int32(env_, 999, &num_val), napi_ok);

    napi_property_descriptor descs[] = {
        {"myConst", nullptr, nullptr, nullptr, nullptr, num_val, napi_default, nullptr},
    };
    ASSERT_EQ(napi_define_properties(env_, obj, 1, descs), napi_ok);

    napi_value read_val;
    ASSERT_EQ(napi_get_named_property(env_, obj, "myConst", &read_val), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, read_val, &val), napi_ok);
    EXPECT_EQ(val, 999);
}

TEST_F(NapiV8Test, DefineObjectAccessors) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    auto getter_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value this_arg;
        napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
        napi_value v;
        napi_get_named_property(env, this_arg, "_backing", &v);
        return v;
    };
    auto setter_cb = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value arg;
        napi_value this_arg;
        napi_get_cb_info(env, info, &argc, &arg, &this_arg, nullptr);
        napi_set_named_property(env, this_arg, "_backing", arg);
        return nullptr;
    };

    napi_property_descriptor descs[] = {
        {"prop", nullptr, nullptr, getter_cb, setter_cb, nullptr, napi_default, nullptr},
    };
    ASSERT_EQ(napi_define_properties(env_, obj, 1, descs), napi_ok);

    napi_value set_val;
    ASSERT_EQ(napi_create_int32(env_, 555, &set_val), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj, "prop", set_val), napi_ok);

    napi_value get_val;
    ASSERT_EQ(napi_get_named_property(env_, obj, "prop", &get_val), napi_ok);
    int32_t val = 0;
    ASSERT_EQ(napi_get_value_int32(env_, get_val, &val), napi_ok);
    EXPECT_EQ(val, 555);
}

TEST_F(NapiV8Test, DefineObjectPropertyAttributes) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    napi_value num_val;
    ASSERT_EQ(napi_create_int32(env_, 42, &num_val), napi_ok);

    napi_property_descriptor descs[] = {
        {"enumProp", nullptr, nullptr, nullptr, nullptr, num_val, napi_enumerable, nullptr},
        {"nonEnumProp", nullptr, nullptr, nullptr, nullptr, num_val, napi_default, nullptr},
    };
    ASSERT_EQ(napi_define_properties(env_, obj, 2, descs), napi_ok);

    // Enumerable property names should contain only "enumProp"
    napi_value enum_names;
    ASSERT_EQ(napi_get_property_names(env_, obj, &enum_names), napi_ok);
    uint32_t enum_len = 0;
    ASSERT_EQ(napi_get_array_length(env_, enum_names, &enum_len), napi_ok);
    EXPECT_EQ(enum_len, 1u);

    napi_value first_name;
    ASSERT_EQ(napi_get_element(env_, enum_names, 0, &first_name), napi_ok);
    char name_buf[32];
    ASSERT_EQ(napi_get_value_string_utf8(env_, first_name, name_buf, sizeof(name_buf), nullptr),
              napi_ok);
    EXPECT_STREQ(name_buf, "enumProp");

    // Both properties can still be accessed directly by name on the object
    napi_value val_enum, val_non_enum;
    ASSERT_EQ(napi_get_named_property(env_, obj, "enumProp", &val_enum), napi_ok);
    ASSERT_EQ(napi_get_named_property(env_, obj, "nonEnumProp", &val_non_enum), napi_ok);
    int32_t v1 = 0, v2 = 0;
    ASSERT_EQ(napi_get_value_int32(env_, val_enum, &v1), napi_ok);
    ASSERT_EQ(napi_get_value_int32(env_, val_non_enum, &v2), napi_ok);
    EXPECT_EQ(v1, 42);
    EXPECT_EQ(v2, 42);
}

TEST_F(NapiV8Test, DefineObjectWritableAttribute) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);

    napi_value val1, val2;
    ASSERT_EQ(napi_create_int32(env_, 100, &val1), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 200, &val2), napi_ok);

    napi_property_descriptor descs[] = {
        {"writableProp", nullptr, nullptr, nullptr, nullptr, val1, napi_writable, nullptr},
        {"readOnlyProp", nullptr, nullptr, nullptr, nullptr, val2, napi_default, nullptr},
    };
    ASSERT_EQ(napi_define_properties(env_, obj, 2, descs), napi_ok);

    // Writable property can be overwritten
    napi_value new_val1;
    ASSERT_EQ(napi_create_int32(env_, 999, &new_val1), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj, "writableProp", new_val1), napi_ok);
    napi_value read_w;
    ASSERT_EQ(napi_get_named_property(env_, obj, "writableProp", &read_w), napi_ok);
    int32_t w_res = 0;
    ASSERT_EQ(napi_get_value_int32(env_, read_w, &w_res), napi_ok);
    EXPECT_EQ(w_res, 999);

    // Read-only property assignment in non-strict JS returns napi_ok but leaves value unchanged
    napi_value new_val2;
    ASSERT_EQ(napi_create_int32(env_, 888, &new_val2), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, obj, "readOnlyProp", new_val2), napi_ok);
    napi_value read_ro;
    ASSERT_EQ(napi_get_named_property(env_, obj, "readOnlyProp", &read_ro), napi_ok);
    int32_t ro_res = 0;
    ASSERT_EQ(napi_get_value_int32(env_, read_ro, &ro_res), napi_ok);
    EXPECT_EQ(ro_res, 200);  // Unchanged!
}

TEST_F(NapiV8Test, DefineObjectEmptyProperties) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    ASSERT_EQ(napi_define_properties(env_, obj, 0, nullptr), napi_ok);
}

// 5. Invalid Arguments
TEST_F(NapiV8Test, FunctionInvalidArgs) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    napi_value fn;
    auto noop_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    ASSERT_EQ(napi_create_function(env_, "noop", NAPI_AUTO_LENGTH, noop_cb, nullptr, &fn), napi_ok);

    // napi_create_function
    EXPECT_EQ(napi_create_function(nullptr, "f", NAPI_AUTO_LENGTH, noop_cb, nullptr, &fn),
              napi_invalid_arg);
    EXPECT_EQ(napi_create_function(env_, "f", NAPI_AUTO_LENGTH, nullptr, nullptr, &fn),
              napi_invalid_arg);
    EXPECT_EQ(napi_create_function(env_, "f", NAPI_AUTO_LENGTH, noop_cb, nullptr, nullptr),
              napi_invalid_arg);

    // napi_call_function
    napi_value call_res;
    EXPECT_EQ(napi_call_function(env_, nullptr, obj, 0, nullptr, &call_res),
              napi_function_expected);
    EXPECT_EQ(napi_call_function(nullptr, nullptr, fn, 0, nullptr, &call_res), napi_invalid_arg);
    EXPECT_EQ(napi_call_function(env_, nullptr, nullptr, 0, nullptr, &call_res), napi_invalid_arg);

    // napi_get_cb_info
    EXPECT_EQ(napi_get_cb_info(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr),
              napi_invalid_arg);
    EXPECT_EQ(napi_get_cb_info(env_, nullptr, nullptr, nullptr, nullptr, nullptr),
              napi_invalid_arg);
}

TEST_F(NapiV8Test, ClassAndPropertyInvalidArgs) {
    napi_value obj;
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    napi_value fn;
    auto noop_cb = [](napi_env, napi_callback_info) -> napi_value { return nullptr; };
    ASSERT_EQ(napi_create_function(env_, "noop", NAPI_AUTO_LENGTH, noop_cb, nullptr, &fn), napi_ok);

    // napi_new_instance
    napi_value inst_res;
    EXPECT_EQ(napi_new_instance(env_, obj, 0, nullptr, &inst_res), napi_function_expected);
    EXPECT_EQ(napi_new_instance(nullptr, fn, 0, nullptr, &inst_res), napi_invalid_arg);
    EXPECT_EQ(napi_new_instance(env_, nullptr, 0, nullptr, &inst_res), napi_invalid_arg);
    EXPECT_EQ(napi_new_instance(env_, fn, 0, nullptr, nullptr), napi_invalid_arg);

    // napi_define_class
    napi_value ctor_res;
    EXPECT_EQ(
        napi_define_class(nullptr, "C", NAPI_AUTO_LENGTH, noop_cb, nullptr, 0, nullptr, &ctor_res),
        napi_invalid_arg);
    EXPECT_EQ(
        napi_define_class(env_, "C", NAPI_AUTO_LENGTH, nullptr, nullptr, 0, nullptr, &ctor_res),
        napi_invalid_arg);
    EXPECT_EQ(napi_define_class(env_, "C", NAPI_AUTO_LENGTH, noop_cb, nullptr, 0, nullptr, nullptr),
              napi_invalid_arg);
    EXPECT_EQ(
        napi_define_class(env_, "C", NAPI_AUTO_LENGTH, noop_cb, nullptr, 1, nullptr, &ctor_res),
        napi_invalid_arg);

    // napi_define_properties
    EXPECT_EQ(napi_define_properties(nullptr, obj, 0, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_define_properties(env_, nullptr, 0, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_define_properties(env_, obj, 1, nullptr), napi_invalid_arg);
}

// ============================================================================
// Stage 5: Errors & Exceptions
// ============================================================================

TEST_F(NapiV8Test, CreateError) {
    napi_value msg;
    ASSERT_EQ(napi_create_string_utf8(env_, "Something went wrong", NAPI_AUTO_LENGTH, &msg),
              napi_ok);

    napi_value err;
    ASSERT_EQ(napi_create_error(env_, nullptr, msg, &err), napi_ok);

    bool is_err = false;
    ASSERT_EQ(napi_is_error(env_, err, &is_err), napi_ok);
    EXPECT_TRUE(is_err);

    napi_value read_msg;
    ASSERT_EQ(napi_get_named_property(env_, err, "message", &read_msg), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_msg, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "Something went wrong");
}

TEST_F(NapiV8Test, CreateErrorWithCode) {
    napi_value code;
    ASSERT_EQ(napi_create_string_utf8(env_, "ERR_CUSTOM_CODE", NAPI_AUTO_LENGTH, &code), napi_ok);
    napi_value msg;
    ASSERT_EQ(napi_create_string_utf8(env_, "Error with code", NAPI_AUTO_LENGTH, &msg), napi_ok);

    napi_value err;
    ASSERT_EQ(napi_create_error(env_, code, msg, &err), napi_ok);

    napi_value read_code;
    ASSERT_EQ(napi_get_named_property(env_, err, "code", &read_code), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_code, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "ERR_CUSTOM_CODE");
}

TEST_F(NapiV8Test, CreateTypeError) {
    napi_value msg;
    ASSERT_EQ(napi_create_string_utf8(env_, "Invalid type provided", NAPI_AUTO_LENGTH, &msg),
              napi_ok);

    napi_value err;
    ASSERT_EQ(napi_create_type_error(env_, nullptr, msg, &err), napi_ok);

    bool is_err = false;
    ASSERT_EQ(napi_is_error(env_, err, &is_err), napi_ok);
    EXPECT_TRUE(is_err);

    napi_value read_name;
    ASSERT_EQ(napi_get_named_property(env_, err, "name", &read_name), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_name, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "TypeError");
}

TEST_F(NapiV8Test, CreateRangeError) {
    napi_value msg;
    ASSERT_EQ(napi_create_string_utf8(env_, "Value out of range", NAPI_AUTO_LENGTH, &msg), napi_ok);

    napi_value err;
    ASSERT_EQ(napi_create_range_error(env_, nullptr, msg, &err), napi_ok);

    bool is_err = false;
    ASSERT_EQ(napi_is_error(env_, err, &is_err), napi_ok);
    EXPECT_TRUE(is_err);

    napi_value read_name;
    ASSERT_EQ(napi_get_named_property(env_, err, "name", &read_name), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_name, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "RangeError");
}

TEST_F(NapiV8Test, IsErrorOnNonErrorTypes) {
    napi_value num, str, b, obj, null_v, undef_v;
    ASSERT_EQ(napi_create_int32(env_, 123, &num), napi_ok);
    ASSERT_EQ(napi_create_string_utf8(env_, "test", NAPI_AUTO_LENGTH, &str), napi_ok);
    ASSERT_EQ(napi_get_boolean(env_, true, &b), napi_ok);
    ASSERT_EQ(napi_create_object(env_, &obj), napi_ok);
    ASSERT_EQ(napi_get_null(env_, &null_v), napi_ok);
    ASSERT_EQ(napi_get_undefined(env_, &undef_v), napi_ok);

    bool is_err = true;
    ASSERT_EQ(napi_is_error(env_, num, &is_err), napi_ok);
    EXPECT_FALSE(is_err);
    ASSERT_EQ(napi_is_error(env_, str, &is_err), napi_ok);
    EXPECT_FALSE(is_err);
    ASSERT_EQ(napi_is_error(env_, b, &is_err), napi_ok);
    EXPECT_FALSE(is_err);
    ASSERT_EQ(napi_is_error(env_, obj, &is_err), napi_ok);
    EXPECT_FALSE(is_err);
    ASSERT_EQ(napi_is_error(env_, null_v, &is_err), napi_ok);
    EXPECT_FALSE(is_err);
    ASSERT_EQ(napi_is_error(env_, undef_v, &is_err), napi_ok);
    EXPECT_FALSE(is_err);
}

TEST_F(NapiV8Test, ThrowAndCatchInCallback) {
    auto throwing_cb = [](napi_env env, napi_callback_info) -> napi_value {
        napi_value msg;
        napi_create_string_utf8(env, "Operation failed", NAPI_AUTO_LENGTH, &msg);
        napi_value err;
        napi_create_error(env, nullptr, msg, &err);
        napi_throw(env, err);
        return nullptr;
    };

    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "throwFn", NAPI_AUTO_LENGTH, throwing_cb, nullptr, &fn),
              napi_ok);

    napi_value call_res;
    EXPECT_EQ(napi_call_function(env_, nullptr, fn, 0, nullptr, &call_res), napi_pending_exception);

    // Verify the caught exception is the exact Error created in the callback
    napi_value caught_err;
    ASSERT_EQ(napi_get_and_clear_last_exception(env_, &caught_err), napi_ok);
    bool is_err = false;
    ASSERT_EQ(napi_is_error(env_, caught_err, &is_err), napi_ok);
    EXPECT_TRUE(is_err);

    napi_value read_msg;
    ASSERT_EQ(napi_get_named_property(env_, caught_err, "message", &read_msg), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_msg, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "Operation failed");
}

TEST_F(NapiV8Test, IsExceptionPendingLifecycle) {
    bool pending = true;
    ASSERT_EQ(napi_is_exception_pending(env_, &pending), napi_ok);
    EXPECT_FALSE(pending);

    auto throwing_cb = [](napi_env env, napi_callback_info) -> napi_value {
        napi_value msg;
        napi_create_string_utf8(env, "Pending error test", NAPI_AUTO_LENGTH, &msg);
        napi_value err;
        napi_create_error(env, nullptr, msg, &err);
        napi_throw(env, err);
        return nullptr;
    };

    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "throwFn", NAPI_AUTO_LENGTH, throwing_cb, nullptr, &fn),
              napi_ok);

    napi_value call_res;
    EXPECT_EQ(napi_call_function(env_, nullptr, fn, 0, nullptr, &call_res), napi_pending_exception);

    ASSERT_EQ(napi_is_exception_pending(env_, &pending), napi_ok);
    EXPECT_TRUE(pending);

    napi_value caught;
    ASSERT_EQ(napi_get_and_clear_last_exception(env_, &caught), napi_ok);

    ASSERT_EQ(napi_is_exception_pending(env_, &pending), napi_ok);
    EXPECT_FALSE(pending);
}

TEST_F(NapiV8Test, GetAndClearLastException) {
    // When no exception has occurred, get_and_clear returns undefined
    napi_value no_err;
    ASSERT_EQ(napi_get_and_clear_last_exception(env_, &no_err), napi_ok);
    napi_valuetype t;
    ASSERT_EQ(napi_typeof(env_, no_err, &t), napi_ok);
    EXPECT_EQ(t, napi_undefined);

    auto throwing_cb = [](napi_env env, napi_callback_info) -> napi_value {
        napi_value msg;
        napi_create_string_utf8(env, "Recoverable error", NAPI_AUTO_LENGTH, &msg);
        napi_value err;
        napi_create_error(env, nullptr, msg, &err);
        napi_throw(env, err);
        return nullptr;
    };

    napi_value fn;
    ASSERT_EQ(napi_create_function(env_, "throwFn", NAPI_AUTO_LENGTH, throwing_cb, nullptr, &fn),
              napi_ok);

    napi_value call_res;
    EXPECT_EQ(napi_call_function(env_, nullptr, fn, 0, nullptr, &call_res), napi_pending_exception);

    napi_value caught;
    ASSERT_EQ(napi_get_and_clear_last_exception(env_, &caught), napi_ok);
    bool is_err = false;
    ASSERT_EQ(napi_is_error(env_, caught, &is_err), napi_ok);
    EXPECT_TRUE(is_err);

    napi_value read_msg;
    ASSERT_EQ(napi_get_named_property(env_, caught, "message", &read_msg), napi_ok);
    char buf[64];
    ASSERT_EQ(napi_get_value_string_utf8(env_, read_msg, buf, sizeof(buf), nullptr), napi_ok);
    EXPECT_STREQ(buf, "Recoverable error");

    // Subsequent call to get_and_clear returns undefined again
    napi_value after_clear;
    ASSERT_EQ(napi_get_and_clear_last_exception(env_, &after_clear), napi_ok);
    ASSERT_EQ(napi_typeof(env_, after_clear, &t), napi_ok);
    EXPECT_EQ(t, napi_undefined);
}

TEST_F(NapiV8Test, ErrorInvalidArguments) {
    napi_value msg, num, err, res;
    ASSERT_EQ(napi_create_string_utf8(env_, "msg", NAPI_AUTO_LENGTH, &msg), napi_ok);
    ASSERT_EQ(napi_create_int32(env_, 123, &num), napi_ok);
    ASSERT_EQ(napi_create_error(env_, nullptr, msg, &err), napi_ok);

    // napi_create_error
    EXPECT_EQ(napi_create_error(nullptr, nullptr, msg, &res), napi_invalid_arg);
    EXPECT_EQ(napi_create_error(env_, nullptr, nullptr, &res), napi_invalid_arg);
    EXPECT_EQ(napi_create_error(env_, nullptr, msg, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_create_error(env_, nullptr, num, &res), napi_string_expected);

    // napi_create_type_error
    EXPECT_EQ(napi_create_type_error(nullptr, nullptr, msg, &res), napi_invalid_arg);
    EXPECT_EQ(napi_create_type_error(env_, nullptr, nullptr, &res), napi_invalid_arg);
    EXPECT_EQ(napi_create_type_error(env_, nullptr, msg, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_create_type_error(env_, nullptr, num, &res), napi_string_expected);

    // napi_create_range_error
    EXPECT_EQ(napi_create_range_error(nullptr, nullptr, msg, &res), napi_invalid_arg);
    EXPECT_EQ(napi_create_range_error(env_, nullptr, nullptr, &res), napi_invalid_arg);
    EXPECT_EQ(napi_create_range_error(env_, nullptr, msg, nullptr), napi_invalid_arg);
    EXPECT_EQ(napi_create_range_error(env_, nullptr, num, &res), napi_string_expected);

    // napi_throw
    EXPECT_EQ(napi_throw(nullptr, err), napi_invalid_arg);
    EXPECT_EQ(napi_throw(env_, nullptr), napi_invalid_arg);

    // napi_is_exception_pending
    bool pending = false;
    EXPECT_EQ(napi_is_exception_pending(nullptr, &pending), napi_invalid_arg);
    EXPECT_EQ(napi_is_exception_pending(env_, nullptr), napi_invalid_arg);

    // napi_get_and_clear_last_exception
    EXPECT_EQ(napi_get_and_clear_last_exception(nullptr, &res), napi_invalid_arg);
    EXPECT_EQ(napi_get_and_clear_last_exception(env_, nullptr), napi_invalid_arg);

    // napi_is_error
    bool is_err = false;
    EXPECT_EQ(napi_is_error(nullptr, err, &is_err), napi_invalid_arg);
    EXPECT_EQ(napi_is_error(env_, nullptr, &is_err), napi_invalid_arg);
    EXPECT_EQ(napi_is_error(env_, err, nullptr), napi_invalid_arg);
}

}  // namespace
