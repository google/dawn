// Copyright 2021 The Tint Authors.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/debug.h"
#include "src/utils/command.h"
#include "src/writer/hlsl/test_helper.h"

namespace {

void TintInternalCompilerErrorReporter(const tint::diag::List& diagnostics) {
  FAIL() << diagnostics.str();
}

struct Flags {
  bool validate_hlsl = false;
  std::string dxc_path;

  bool parse(int argc, char** argv) {
    bool errored = false;
    for (int i = 1; i < argc && !errored; i++) {
      auto match = [&](std::string name) { return name == argv[i]; };

      auto parse_value = [&](std::string name, std::string& value) {
        if (!match(name)) {
          return false;
        }
        if (i + 1 >= argc) {
          std::cout << "Expected value for flag " << name << "" << std::endl;
          errored = true;
          return false;
        }
        i++;
        value = argv[i];
        return true;
      };

      if (match("--validate-hlsl") || parse_value("--dxc-path", dxc_path)) {
        validate_hlsl = true;
      } else {
        std::cout << "Unknown flag '" << argv[i] << "'" << std::endl;
        return false;
      }
    }
    return true;
  }
};

}  // namespace

// Entry point for tint unit tests
int main(int argc, char** argv) {
  testing::InitGoogleMock(&argc, argv);

  Flags flags;
  if (!flags.parse(argc, argv)) {
    return -1;
  }

#if TINT_BUILD_HLSL_WRITER
  // This must be kept alive for the duration of RUN_ALL_TESTS() as the c_str()
  // is passed into tint::writer::hlsl::EnableHLSLValidation(), which does not
  // make a copy. This is to work around Chromium's strict rules on globals
  // having no constructors / destructors.
  std::string dxc_path;
  if (flags.validate_hlsl) {
    auto dxc = flags.dxc_path.empty() ? tint::utils::Command::LookPath("dxc")
                                      : tint::utils::Command(flags.dxc_path);

    if (!dxc.Found()) {
      std::cout << "DXC executable not found" << std::endl;
      return -1;
    }

    std::cout << "HLSL validation with DXC enabled" << std::endl;

    dxc_path = dxc.Path();
    tint::writer::hlsl::EnableHLSLValidation(dxc_path.c_str());
  } else {
    std::cout << "HLSL validation with DXC is not enabled" << std::endl;
  }
#endif  // TINT_BUILD_HLSL_WRITER

  tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

  auto res = RUN_ALL_TESTS();

  return res;
}
