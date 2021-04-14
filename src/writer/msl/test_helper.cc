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

#include "src/writer/msl/test_helper.h"

#include "src/utils/command.h"
#include "src/utils/tmpfile.h"

namespace tint {
namespace writer {
namespace msl {

namespace {

const char* xcrun_path = nullptr;

}  // namespace

void EnableMSLValidation(const char* xcrun) {
  xcrun_path = xcrun;
}

CompileResult Compile(Program* program) {
  CompileResult result;

  if (!xcrun_path) {
    result.status = CompileResult::Status::kVerificationNotEnabled;
    return result;
  }

  auto xcrun = utils::Command(xcrun_path);
  if (!xcrun.Found()) {
    result.output = "xcrun not found at '" + std::string(xcrun_path) + "'";
    result.status = CompileResult::Status::kFailed;
    return result;
  }

  auto gen = std::make_unique<GeneratorImpl>(program);
  if (!gen->Generate()) {
    result.output = gen->error();
    result.status = CompileResult::Status::kFailed;
    return result;
  }
  result.msl = gen->result();

  utils::TmpFile file(".metal");
  file << result.msl;

  auto xcrun_res =
      xcrun("-sdk", "macosx", "metal", "-o", "/dev/null", "-c", file.Path());
  if (!xcrun_res.out.empty()) {
    if (!result.output.empty()) {
      result.output += "\n";
    }
    result.output += xcrun_res.out;
  }
  if (!xcrun_res.err.empty()) {
    if (!result.output.empty()) {
      result.output += "\n";
    }
    result.output += xcrun_res.err;
  }
  result.status = (xcrun_res.error_code == 0) ? CompileResult::Status::kSuccess
                                              : CompileResult::Status::kFailed;

  return result;
}

}  // namespace msl
}  // namespace writer
}  // namespace tint
