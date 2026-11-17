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

#include "src/tint/utils/containers/function_ref.h"

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "gtest/gtest.h"

namespace tint {
namespace {

int Add(int a, int b) {
    return a + b;
}

/// A callable that counts the number of times it has been called.
struct Counter {
    int operator()() { return ++count; }
    int count = 0;
};

/// A callable that can only be called through a const reference.
struct ConstCallable {
    int operator()() const { return value; }
    int operator()() = delete;
    int value = 0;
};

/// A type that counts its copies and moves.
struct CopyCounter {
    CopyCounter() = default;
    CopyCounter(const CopyCounter& other) : copies(other.copies + 1), moves(other.moves) {}
    CopyCounter(CopyCounter&& other) : copies(other.copies), moves(other.moves + 1) {}
    int copies = 0;
    int moves = 0;
};

struct Base {
    int value = 0;
};
struct Derived : Base {};

struct Object {
    int Get() const { return value; }
    int value = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Compile-time checks
////////////////////////////////////////////////////////////////////////////////
using IntToInt = FunctionRef<int(int)>;
using IntLambda = decltype([](int i) { return i; });

static_assert(std::is_trivially_copyable_v<IntToInt>);
static_assert(std::is_constructible_v<IntToInt, IntLambda>);
static_assert(!std::is_constructible_v<IntToInt, decltype([](int, int) { return 0; })>,
              "wrong number of parameters");
static_assert(!std::is_constructible_v<IntToInt, decltype([](std::string) { return 0; })>,
              "incompatible parameter type");
static_assert(!std::is_constructible_v<IntToInt, decltype([](int) { return std::string{}; })>,
              "incompatible return type");
static_assert(!std::is_constructible_v<IntToInt, std::nullptr_t>, "cannot reference null");

// Functions and function pointers would need casting to be stored, so must be wrapped in a lambda.
static_assert(!std::is_constructible_v<FunctionRef<int(int, int)>, decltype(Add)&>);
static_assert(!std::is_constructible_v<FunctionRef<int(int, int)>, decltype(&Add)>);
static_assert(!std::is_constructible_v<FunctionRef<int(int, int)>, decltype(&Add)&>);
static_assert(!std::is_default_constructible_v<IntToInt>, "there is no empty state");

// A reference must not be returned to a temporary created by the conversion.
static_assert(!std::is_constructible_v<FunctionRef<const int&()>, decltype([] { return 1; })>);
static_assert(!std::is_constructible_v<FunctionRef<const int64_t&()>, decltype([]() -> const int& {
                                           static int i = 0;
                                           return i;
                                       })>);
static_assert(std::is_constructible_v<FunctionRef<const int&()>, decltype([]() -> int& {
                                          static int i = 0;
                                          return i;
                                      })>);
static_assert(std::is_constructible_v<FunctionRef<const Base&()>, decltype([]() -> Derived& {
                                          static Derived d;
                                          return d;
                                      })>);

// Assigning a callable would leave the FunctionRef referencing a destroyed temporary.
static_assert(std::is_copy_assignable_v<IntToInt>);
static_assert(!std::is_assignable_v<IntToInt&, IntLambda>);
static_assert(!std::is_assignable_v<IntToInt&, IntLambda&>);

////////////////////////////////////////////////////////////////////////////////
// Runtime tests
////////////////////////////////////////////////////////////////////////////////
TEST(FunctionRefTest, Size) {
    EXPECT_EQ(sizeof(FunctionRef<void()>), sizeof(void*) + sizeof(void (*)()));
}

TEST(FunctionRefTest, Lambda) {
    auto lambda = [](int i) { return i * 2; };
    FunctionRef<int(int)> ref = lambda;
    EXPECT_EQ(ref(3), 6);
    EXPECT_EQ(ref(-4), -8);
}

TEST(FunctionRefTest, LambdaCapture) {
    int total = 0;
    auto lambda = [&](int i) { total += i; };
    FunctionRef<void(int)> ref = lambda;
    ref(1);
    ref(2);
    EXPECT_EQ(total, 3);
}

TEST(FunctionRefTest, LambdaTemporaryAsArgument) {
    auto call = [](FunctionRef<int(int)> fn, int i) { return fn(i); };
    int captured = 10;
    EXPECT_EQ(call([&](int i) { return i + captured; }, 5), 15);
}

TEST(FunctionRefTest, MutableLambda) {
    auto lambda = [n = 0]() mutable { return ++n; };
    FunctionRef<int()> ref = lambda;
    EXPECT_EQ(ref(), 1);
    EXPECT_EQ(ref(), 2);
    // The lambda is referenced, not copied, so it holds the updated state.
    EXPECT_EQ(lambda(), 3);
}

TEST(FunctionRefTest, CallableObjectIsReferenced) {
    Counter counter;
    FunctionRef<int()> ref = counter;
    EXPECT_EQ(ref(), 1);
    EXPECT_EQ(ref(), 2);
    EXPECT_EQ(counter.count, 2);
}

TEST(FunctionRefTest, ConstCallableObject) {
    const ConstCallable callable{42};
    FunctionRef<int()> ref = callable;
    EXPECT_EQ(ref(), 42);
}

TEST(FunctionRefTest, FunctionWrappedInLambda) {
    auto add = [](int a, int b) { return Add(a, b); };
    FunctionRef<int(int, int)> ref = add;
    EXPECT_EQ(ref(2, 3), 5);
}

TEST(FunctionRefTest, MemberFunctionPointer) {
    auto get = &Object::Get;
    FunctionRef<int(const Object&)> ref = get;
    EXPECT_EQ(ref(Object{7}), 7);
}

TEST(FunctionRefTest, MemberDataPointer) {
    auto member = &Object::value;
    FunctionRef<int(const Object&)> ref = member;
    EXPECT_EQ(ref(Object{9}), 9);
}

TEST(FunctionRefTest, ArgumentByLvalueReference) {
    auto lambda = [](int& i) { i = 42; };
    FunctionRef<void(int&)> ref = lambda;
    int value = 0;
    ref(value);
    EXPECT_EQ(value, 42);
}

TEST(FunctionRefTest, ArgumentByConstReferenceIsNotCopied) {
    auto lambda = [](const CopyCounter& c) { return c.copies + c.moves; };
    FunctionRef<int(const CopyCounter&)> ref = lambda;
    CopyCounter c;
    EXPECT_EQ(ref(c), 0);
}

TEST(FunctionRefTest, ArgumentByRvalueReference) {
    std::unique_ptr<int> received;
    auto lambda = [&](std::unique_ptr<int>&& p) { received = std::move(p); };
    FunctionRef<void(std::unique_ptr<int>&&)> ref = lambda;
    ref(std::make_unique<int>(5));
    ASSERT_NE(received, nullptr);
    EXPECT_EQ(*received, 5);
}

TEST(FunctionRefTest, ArgumentByValueIsMovedNotCopied) {
    auto lambda = [](CopyCounter c) { return c.copies; };
    FunctionRef<int(CopyCounter)> ref = lambda;
    EXPECT_EQ(ref(CopyCounter{}), 0);
}

TEST(FunctionRefTest, MoveOnlyArgumentByValue) {
    auto lambda = [](std::unique_ptr<int> p) { return *p; };
    FunctionRef<int(std::unique_ptr<int>)> ref = lambda;
    EXPECT_EQ(ref(std::make_unique<int>(3)), 3);
}

TEST(FunctionRefTest, ReturnsReference) {
    int value = 1;
    auto lambda = [&]() -> int& { return value; };
    FunctionRef<int&()> ref = lambda;
    ref() = 2;
    EXPECT_EQ(value, 2);
    EXPECT_EQ(&ref(), &value);
}

TEST(FunctionRefTest, ReturnsBaseReferenceToDerived) {
    Derived derived;
    derived.value = 3;
    auto lambda = [&]() -> Derived& { return derived; };
    FunctionRef<const Base&()> ref = lambda;
    EXPECT_EQ(&ref(), static_cast<Base*>(&derived));
}

TEST(FunctionRefTest, ReturnsMoveOnly) {
    auto lambda = [] { return std::make_unique<int>(4); };
    FunctionRef<std::unique_ptr<int>()> ref = lambda;
    auto p = ref();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, 4);
}

TEST(FunctionRefTest, ReturnValueIsConverted) {
    auto lambda = [] { return 3; };
    FunctionRef<double()> ref = lambda;
    EXPECT_EQ(ref(), 3.0);
}

TEST(FunctionRefTest, VoidDiscardsReturnValue) {
    int calls = 0;
    auto lambda = [&] { return ++calls; };
    FunctionRef<void()> ref = lambda;
    ref();
    EXPECT_EQ(calls, 1);
}

TEST(FunctionRefTest, CopyReferencesSameCallable) {
    Counter counter;
    FunctionRef<int()> a = counter;
    FunctionRef<int()> b = a;
    a();
    b();
    EXPECT_EQ(counter.count, 2);
}

TEST(FunctionRefTest, CopyAssignment) {
    Counter first;
    Counter second;
    FunctionRef<int()> a = first;
    FunctionRef<int()> b = second;
    a = b;
    a();
    EXPECT_EQ(first.count, 0);
    EXPECT_EQ(second.count, 1);
}

TEST(FunctionRefTest, ConstFunctionRefCanBeCalled) {
    auto lambda = [] { return 1; };
    const FunctionRef<int()> ref = lambda;
    EXPECT_EQ(ref(), 1);
}

TEST(FunctionRefTest, FunctionRefOfDifferentSignature) {
    auto lambda = [](int i) { return i + 1; };
    FunctionRef<int(int)> inner = lambda;
    FunctionRef<int64_t(int16_t)> outer = inner;
    EXPECT_EQ(outer(1), 2);
}

}  // namespace
}  // namespace tint
