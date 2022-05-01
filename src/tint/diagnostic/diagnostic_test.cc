// Copyright 2020 The Tint Authors.
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

#include "src/tint/diagnostic/formatter.h"

#include "gtest/gtest.h"
#include "src/tint/diagnostic/diagnostic.h"

namespace tint::diag {
namespace {

TEST(DiagListTest, OwnedFilesShared) {
    auto file = std::make_shared<Source::File>("path", "content");

    diag::List list_a, list_b;
    {
        diag::Diagnostic diag{};
        diag.source = Source{Source::Range{{0, 0}}, file.get()};
        list_a.add(std::move(diag));
    }

    list_b = list_a;

    ASSERT_EQ(list_b.count(), list_a.count());
    EXPECT_EQ(list_b.begin()->source.file, file.get());
}

}  // namespace
}  // namespace tint::diag
