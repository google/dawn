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

#include "src/writer/hlsl/test_helper.h"

#include "src/utils/command.h"
#include "src/utils/tmpfile.h"

namespace tint {
namespace writer {
namespace hlsl {

namespace {

const char* dxc_path = nullptr;

}  // namespace

void EnableHLSLValidation(const char* dxc) {
  dxc_path = dxc;
}

CompileResult Compile(Program* program, GeneratorImpl* generator) {
  CompileResult result;

  if (!dxc_path) {
    result.status = CompileResult::Status::kVerificationNotEnabled;
    return result;
  }

  auto dxc = utils::Command(dxc_path);
  if (!dxc.Found()) {
    result.output = "DXC not found at '" + std::string(dxc_path) + "'";
    result.status = CompileResult::Status::kFailed;
    return result;
  }

  std::ostringstream hlsl;
  if (!generator->Generate(hlsl)) {
    result.output = generator->error();
    result.status = CompileResult::Status::kFailed;
    return result;
  }
  result.hlsl = hlsl.str();

  utils::TmpFile file;
  file << result.hlsl;

  bool found_an_entrypoint = false;
  for (auto* func : program->AST().Functions()) {
    if (func->IsEntryPoint()) {
      found_an_entrypoint = true;

      const char* profile = "";

      switch (func->pipeline_stage()) {
        case ast::PipelineStage::kNone:
          result.output = "Invalid PipelineStage";
          result.status = CompileResult::Status::kFailed;
          return result;
        case ast::PipelineStage::kVertex:
          profile = "-T vs_6_0";
          break;
        case ast::PipelineStage::kFragment:
          profile = "-T ps_6_0";
          break;
        case ast::PipelineStage::kCompute:
          profile = "-T cs_6_0";
          break;
      }

      auto name = program->Symbols().NameFor(func->symbol());
      auto res = dxc(profile, "-E " + name, file.Path());
      if (!res.out.empty()) {
        if (!result.output.empty()) {
          result.output += "\n";
        }
        result.output += res.out;
      }
      if (!res.err.empty()) {
        if (!result.output.empty()) {
          result.output += "\n";
        }
        result.output += res.err;
      }
      result.status = (res.error_code == 0) ? CompileResult::Status::kSuccess
                                            : CompileResult::Status::kFailed;
    }
  }

  if (!found_an_entrypoint) {
    result.output = "No entrypoint found";
    result.status = CompileResult::Status::kFailed;
    return result;
  }

  return result;
}

}  // namespace hlsl
}  // namespace writer
}  // namespace tint
