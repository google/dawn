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

#include <gtest/gtest.h>

#include "backend/RefCounted.h"

using namespace backend;

struct RCTest : public RefCounted {
    RCTest() {
    }

    RCTest(bool* deleted): deleted(deleted) {
    }

    ~RCTest() override {
        if (deleted != nullptr) {
            *deleted = true;
        }
    }

    RCTest* GetThis() {
        return this;
    }

    bool* deleted = nullptr;
};

// Test that RCs start with one external ref, and removing it destroys the object.
TEST(RefCounted, StartsWithOneExternalRef) {
    bool deleted = false;
    auto test = new RCTest(&deleted);

    test->Release();
    ASSERT_TRUE(deleted);
}

// Test internal refs keep the RC alive.
TEST(RefCounted, InternalRefKeepsAlive) {
    bool deleted = false;
    auto test = new RCTest(&deleted);

    test->ReferenceInternal();
    test->Release();
    ASSERT_FALSE(deleted);

    test->ReleaseInternal();
    ASSERT_TRUE(deleted);
}

// Test that when adding an external ref from 0, an internal ref is added
TEST(RefCounted, AddExternalRefFromZero) {
    bool deleted = false;
    auto test = new RCTest(&deleted);

    test->ReferenceInternal();
    test->Release();
    ASSERT_FALSE(deleted);

    // Reference adds an internal ref and release removes one
    test->Reference();
    test->Release();
    ASSERT_FALSE(deleted);

    test->ReleaseInternal();
    ASSERT_TRUE(deleted);
}

// Test Ref remove internal reference when going out of scope
TEST(Ref, EndOfScopeRemovesInternalRef) {
    bool deleted = false;
    {
        Ref<RCTest> test(new RCTest(&deleted));
        test->Release();
    }
    ASSERT_TRUE(deleted);
}

// Test getting pointer out of the Ref
TEST(Ref, Gets) {
    RCTest* original = new RCTest;
    Ref<RCTest> test(original);
    test->Release();

    ASSERT_EQ(test.Get(), original);
    ASSERT_EQ(&*test, original);
    ASSERT_EQ(test->GetThis(), original);
}

// Test Refs default to null
TEST(Ref, DefaultsToNull) {
    Ref<RCTest> test;

    ASSERT_EQ(test.Get(), nullptr);
    ASSERT_EQ(&*test, nullptr);
    ASSERT_EQ(test->GetThis(), nullptr);
}

// Test Refs can be used inside ifs
TEST(Ref, BoolConversion) {
    Ref<RCTest> empty;
    Ref<RCTest> full(new RCTest);
    full->Release();

    if (!full || empty) {
        ASSERT_TRUE(false);
    }
}

// Test Ref's copy constructor
TEST(Ref, CopyConstructor) {
    bool deleted = false;
    RCTest* original = new RCTest(&deleted);

    Ref<RCTest> source(original);
    Ref<RCTest> destination(source);
    original->Release();

    ASSERT_EQ(source.Get(), original);
    ASSERT_EQ(destination.Get(), original);

    source = nullptr;
    ASSERT_FALSE(deleted);
    destination = nullptr;
    ASSERT_TRUE(deleted);
}

// Test Ref's copy assignment
TEST(Ref, CopyAssignment) {
    bool deleted = false;
    RCTest* original = new RCTest(&deleted);

    Ref<RCTest> source(original);
    original->Release();

    Ref<RCTest> destination;
    destination = source;

    ASSERT_EQ(source.Get(), original);
    ASSERT_EQ(destination.Get(), original);

    source = nullptr;
    // This fails when address sanitizer is turned on
    ASSERT_FALSE(deleted);

    destination = nullptr;
    ASSERT_TRUE(deleted);
}

// Test Ref's move constructor
TEST(Ref, MoveConstructor) {
    bool deleted = false;
    RCTest* original = new RCTest(&deleted);

    Ref<RCTest> source(original);
    Ref<RCTest> destination(std::move(source));
    original->Release();

    ASSERT_EQ(source.Get(), nullptr);
    ASSERT_EQ(destination.Get(), original);
    ASSERT_FALSE(deleted);

    destination = nullptr;
    ASSERT_TRUE(deleted);
}

// Test Ref's move assignment
TEST(Ref, MoveAssignment) {
    bool deleted = false;
    RCTest* original = new RCTest(&deleted);

    Ref<RCTest> source(original);
    original->Release();

    Ref<RCTest> destination;
    destination = std::move(source);

    ASSERT_EQ(source.Get(), nullptr);
    ASSERT_EQ(destination.Get(), original);
    ASSERT_FALSE(deleted);

    destination = nullptr;
    ASSERT_TRUE(deleted);
}
