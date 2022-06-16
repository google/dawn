// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/remove_continue_in_switch.h"
#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using RemoveContinueInSwitchTest = TransformTest;

TEST_F(RemoveContinueInSwitchTest, ShouldRun_True) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    break;
  }
}
)";

    EXPECT_TRUE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, ShouldRunEmptyModule_False) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, ShouldRunContinueNotInSwitch_False) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    switch(i) {
      case 0: {
        break;
      }
      default: {
        break;
      }
    }

    if (true) {
      continue;
    }
    break;
  }
}
)";

    EXPECT_FALSE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, ShouldRunContinueInLoopInSwitch_False) {
    auto* src = R"(
fn f() {
  var i = 0;
  switch(i) {
    case 0: {
      loop {
        if (true) {
          continue;
        }
        break;
      }
      break;
    }
    default: {
      break;
    }
  }
}
)";

    EXPECT_FALSE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, SingleContinue) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    var tint_continue : bool = false;
    switch(i) {
      case 0: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, MultipleContinues) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      case 1: {
        continue;
        break;
      }
      case 2: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    var tint_continue : bool = false;
    switch(i) {
      case 0: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      case 1: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      case 2: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, MultipleSwitch) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;

    let marker3 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker4 = 0;

    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    var tint_continue : bool = false;
    switch(i) {
      case 0: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    let marker3 = 0;
    var tint_continue_1 : bool = false;
    switch(i) {
      case 0: {
        {
          tint_continue_1 = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue_1) {
      continue;
    }
    let marker4 = 0;
    break;
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitch) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        var j = 0;
        loop {
          let marker3 = 0;
          switch(j) {
            case 0: {
              continue;
              break;
            }
            default: {
              break;
            }
          }
          let marker4 = 0;
          break;
        }
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    var tint_continue_1 : bool = false;
    switch(i) {
      case 0: {
        var j = 0;
        loop {
          let marker3 = 0;
          var tint_continue : bool = false;
          switch(j) {
            case 0: {
              {
                tint_continue = true;
                break;
              }
              break;
            }
            default: {
              break;
            }
          }
          if (tint_continue) {
            continue;
          }
          let marker4 = 0;
          break;
        }
        {
          tint_continue_1 = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue_1) {
      continue;
    }
    let marker2 = 0;
    break;
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, ExtraScopes) {
    auto* src = R"(
fn f() {
  var i = 0;
  var a = true;
  var b = true;
  var c = true;
  var d = true;
  loop {
    if (a) {
      if (b) {
        let marker1 = 0;
        switch(i) {
          case 0: {
            if (c) {
              if (d) {
                continue;
              }
            }
            break;
          }
          default: {
            break;
          }
        }
        let marker2 = 0;
        break;
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  var a = true;
  var b = true;
  var c = true;
  var d = true;
  loop {
    if (a) {
      if (b) {
        let marker1 = 0;
        var tint_continue : bool = false;
        switch(i) {
          case 0: {
            if (c) {
              if (d) {
                {
                  tint_continue = true;
                  break;
                }
              }
            }
            break;
          }
          default: {
            break;
          }
        }
        if (tint_continue) {
          continue;
        }
        let marker2 = 0;
        break;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, ForLoop) {
    auto* src = R"(
fn f() {
  for (var i = 0; i < 4; i = i + 1) {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  for(var i = 0; (i < 4); i = (i + 1)) {
    let marker1 = 0;
    var tint_continue : bool = false;
    switch(i) {
      case 0: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, While) {
    auto* src = R"(
fn f() {
  var i = 0;
  while (i < 4) {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  while((i < 4)) {
    let marker1 = 0;
    var tint_continue : bool = false;
    switch(i) {
      case 0: {
        {
          tint_continue = true;
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;
  }
}
)";

    DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
