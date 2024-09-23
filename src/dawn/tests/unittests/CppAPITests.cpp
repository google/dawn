// Copyright 2024 The Dawn & Tint Authors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dawn/native/dawn_platform.h"

namespace dawn::native {
namespace {

// Test that default construction or assignment to wgpu::NullableStringView produces the nil string.
TEST(CppAPITests, WGPUStringDefault) {
    {
        wgpu::NullableStringView s;
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s{};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = {};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = wgpu::NullableStringView();
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that resetting the string, clears both data and length.
    std::string_view sv("hello world!");
    {
        wgpu::NullableStringView s(sv);
        s = {};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s(sv);
        s = wgpu::NullableStringView();
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that construction or assignment to wgpu::NullableStringView from const char*.
TEST(CppAPITests, WGPUStringFromCstr) {
    {
        wgpu::NullableStringView s("hello world!");
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s{"hello world!"};
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = {"hello world!"};
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = wgpu::NullableStringView("hello world!");
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that setting to a cstr clears the length.
    std::string_view sv("hello world!");
    {
        wgpu::NullableStringView s(sv);
        s = "other str";
        EXPECT_STREQ(s.data, "other str");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that construction or assignment to wgpu::NullableStringView from std::string_view
TEST(CppAPITests, WGPUStringFromStdStringView) {
    std::string_view sv("hello\x00world!");
    {
        wgpu::NullableStringView s(sv);
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::NullableStringView s{sv};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::NullableStringView s = {sv};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::NullableStringView s = wgpu::NullableStringView(sv);
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
}

// Test that construction or assignment to wgpu::NullableStringView from pointer and length
TEST(CppAPITests, WGPUStringFromPtrAndLength) {
    std::string_view sv("hello\x00world!");
    {
        wgpu::NullableStringView s(sv.data(), sv.length());
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::NullableStringView s{sv.data(), sv.length()};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::NullableStringView s = {sv.data(), sv.length()};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::NullableStringView s = wgpu::NullableStringView(sv.data(), sv.length());
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
}

// Test that construction or assignment to wgpu::NullableStringView from nullptr
TEST(CppAPITests, WGPUStringFromNullptr) {
    {
        wgpu::NullableStringView s(nullptr);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s{nullptr};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = {nullptr};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = wgpu::NullableStringView(nullptr);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that setting to nullptr, clears both data and length.
    std::string_view sv("hello world!");
    {
        wgpu::NullableStringView s(sv);
        s = nullptr;
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that construction or assignment to wgpu::NullableStringView from std::nullopt
TEST(CppAPITests, WGPUStringFromNullopt) {
    {
        wgpu::NullableStringView s(std::nullopt);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s{std::nullopt};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = {std::nullopt};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::NullableStringView s = wgpu::NullableStringView(std::nullopt);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that setting to std::nullopt, clears both data and length.
    std::string_view sv("hello world!");
    {
        wgpu::NullableStringView s(sv);
        s = std::nullopt;
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

}  // anonymous namespace
}  // namespace dawn::native
