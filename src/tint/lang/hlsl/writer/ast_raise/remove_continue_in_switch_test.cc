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

#include "src/tint/lang/hlsl/writer/ast_raise/remove_continue_in_switch.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::hlsl::writer {
namespace {

using RemoveContinueInSwitchTest = ast::transform::TransformTest;

TEST_F(RemoveContinueInSwitchTest, ShouldRun_True) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    switch(i) {
      case 0: {
        continue;
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

    ast::transform::DataMap data;
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
  var tint_continue : bool;
  loop {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
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

    ast::transform::DataMap data;
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
      }
      case 1: {
        continue;
      }
      case 2: {
        continue;
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
  var tint_continue : bool;
  loop {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
      }
      case 1: {
        tint_continue = true;
        break;
      }
      case 2: {
        tint_continue = true;
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

    ast::transform::DataMap data;
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
  var tint_continue : bool;
  loop {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
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
    switch(i) {
      case 0: {
        tint_continue = true;
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
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitchSwitch) {
    auto* src = R"(
fn f() {
  var j = 0;
  for (var i = 0; i < 2; i += 2) {
    switch(i) {
      case 0: {
        switch(j) {
          case 0: {
            continue;
          }
          default: {
          }
        }
      }
      default: {
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var j = 0;
  var tint_continue : bool;
  for(var i = 0; (i < 2); i += 2) {
    tint_continue = false;
    switch(i) {
      case 0: {
        switch(j) {
          case 0: {
            tint_continue = true;
            break;
          }
          default: {
          }
        }
        if (tint_continue) {
          break;
        }
      }
      default: {
      }
    }
    if (tint_continue) {
      continue;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopLoopSwitch) {
    auto* src = R"(
fn f() {
  for (var i = 0; i < 2; i += 2) {
    for (var j = 0; j < 2; j += 2) {
      switch(i) {
        case 0: {
          continue;
        }
        default: {
        }
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  for(var i = 0; (i < 2); i += 2) {
    var tint_continue : bool;
    for(var j = 0; (j < 2); j += 2) {
      tint_continue = false;
      switch(i) {
        case 0: {
          tint_continue = true;
          break;
        }
        default: {
        }
      }
      if (tint_continue) {
        continue;
      }
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitchLoopSwitch) {
    auto* src = R"(
fn f() {
  for (var i = 0; i < 2; i += 2) {
    switch(i) {
      case 0: {
        for (var j = 0; j < 2; j += 2) {
          switch(j) {
            case 0: {
              continue; // j loop
            }
            default: {
            }
          }
        }
        continue; // i loop
      }
      default: {
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var tint_continue_1 : bool;
  for(var i = 0; (i < 2); i += 2) {
    tint_continue_1 = false;
    switch(i) {
      case 0: {
        var tint_continue : bool;
        for(var j = 0; (j < 2); j += 2) {
          tint_continue = false;
          switch(j) {
            case 0: {
              tint_continue = true;
              break;
            }
            default: {
            }
          }
          if (tint_continue) {
            continue;
          }
        }
        tint_continue_1 = true;
        break;
      }
      default: {
      }
    }
    if (tint_continue_1) {
      continue;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitchLoopSwitchSwitch) {
    auto* src = R"(
fn f() {
  var k = 0;
  for (var i = 0; i < 2; i += 2) {
    switch(i) {
      case 0: {
        for (var j = 0; j < 2; j += 2) {
          switch(j) {
            case 0: {
              continue; // j loop
            }
            case 1: {
              switch (k) {
                case 0: {
                  continue; // j loop
                }
                default: {
                }
              }
            }
            default: {
            }
          }
        }
        continue; // i loop
      }
      default: {
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var k = 0;
  var tint_continue_1 : bool;
  for(var i = 0; (i < 2); i += 2) {
    tint_continue_1 = false;
    switch(i) {
      case 0: {
        var tint_continue : bool;
        for(var j = 0; (j < 2); j += 2) {
          tint_continue = false;
          switch(j) {
            case 0: {
              tint_continue = true;
              break;
            }
            case 1: {
              switch(k) {
                case 0: {
                  tint_continue = true;
                  break;
                }
                default: {
                }
              }
              if (tint_continue) {
                break;
              }
            }
            default: {
            }
          }
          if (tint_continue) {
            continue;
          }
        }
        tint_continue_1 = true;
        break;
      }
      default: {
      }
    }
    if (tint_continue_1) {
      continue;
    }
  }
}
)";

    ast::transform::DataMap data;
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
  var tint_continue : bool;
  loop {
    tint_continue = false;
    if (a) {
      if (b) {
        let marker1 = 0;
        switch(i) {
          case 0: {
            if (c) {
              if (d) {
                tint_continue = true;
                break;
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

    ast::transform::DataMap data;
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
  var tint_continue : bool;
  for(var i = 0; (i < 4); i = (i + 1)) {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
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

    ast::transform::DataMap data;
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
  var tint_continue : bool;
  while((i < 4)) {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
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

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::hlsl::writer
