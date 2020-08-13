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

#include <gtest/gtest.h>

#include "common/RefCounted.h"
#include "dawn_native/ToBackend.h"

#include <type_traits>

// Make our own Base - Backend object pair, reusing the AdapterBase name
namespace dawn_native {
    class AdapterBase : public RefCounted {};
}  // namespace dawn_native

using namespace dawn_native;

class MyAdapter : public AdapterBase {};

struct MyBackendTraits {
    using AdapterType = MyAdapter;
};

// Instanciate ToBackend for our "backend"
template <typename T>
auto ToBackend(T&& common) -> decltype(ToBackendBase<MyBackendTraits>(common)) {
    return ToBackendBase<MyBackendTraits>(common);
}

// Test that ToBackend correctly converts pointers to base classes.
TEST(ToBackend, Pointers) {
    {
        MyAdapter* adapter = new MyAdapter;
        const AdapterBase* base = adapter;

        auto backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(backendAdapter), const MyAdapter*>::value, "");
        ASSERT_EQ(adapter, backendAdapter);

        adapter->Release();
    }
    {
        MyAdapter* adapter = new MyAdapter;
        AdapterBase* base = adapter;

        auto backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(backendAdapter), MyAdapter*>::value, "");
        ASSERT_EQ(adapter, backendAdapter);

        adapter->Release();
    }
}

// Test that ToBackend correctly converts Refs to base classes.
TEST(ToBackend, Ref) {
    {
        MyAdapter* adapter = new MyAdapter;
        const Ref<AdapterBase> base(adapter);

        const auto& backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(ToBackend(base)), const Ref<MyAdapter>&>::value, "");
        ASSERT_EQ(adapter, backendAdapter.Get());

        adapter->Release();
    }
    {
        MyAdapter* adapter = new MyAdapter;
        Ref<AdapterBase> base(adapter);

        auto backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(ToBackend(base)), Ref<MyAdapter>&>::value, "");
        ASSERT_EQ(adapter, backendAdapter.Get());

        adapter->Release();
    }
}
