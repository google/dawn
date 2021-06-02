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

#include "src/val/val.h"

#include "src/ast/module.h"
#include "src/program.h"
#include "src/utils/io/command.h"
#include "src/utils/io/tmpfile.h"

namespace tint {
namespace val {

Result Hlsl(const std::string& dxc_path,
            const std::string& source,
            Program* program) {
  Result result;

  auto dxc = utils::Command(dxc_path);
  if (!dxc.Found()) {
    result.output = "DXC not found at '" + std::string(dxc_path) + "'";
    result.failed = true;
    return result;
  }

  result.source = source;

  utils::TmpFile file;
  file << source;

  bool found_an_entrypoint = false;
  for (auto* func : program->AST().Functions()) {
    if (func->IsEntryPoint()) {
      found_an_entrypoint = true;

      const char* profile = "";

      switch (func->pipeline_stage()) {
        case ast::PipelineStage::kNone:
          result.output = "Invalid PipelineStage";
          result.failed = true;
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
      result.failed = (res.error_code != 0);
    }
  }

  if (!found_an_entrypoint) {
    result.output = "No entrypoint found";
    result.failed = true;
    return result;
  }

  return result;
}

Result Msl(const std::string& xcrun_path, const std::string& source) {
  Result result;

  auto xcrun = utils::Command(xcrun_path);
  if (!xcrun.Found()) {
    result.output = "xcrun not found at '" + std::string(xcrun_path) + "'";
    result.failed = true;
    return result;
  }

  result.source = source;

  utils::TmpFile file(".metal");
  file << result.source;

#ifdef _WIN32
  // On Windows, we should actually be running metal.exe from the Metal
  // Developer Tools for Windows
  auto res = xcrun("-x", "metal", "-c", "-o", "NUL", file.Path());
#else
  auto res =
      xcrun("-sdk", "macosx", "metal", "-o", "/dev/null", "-c", file.Path());
#endif
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
  result.failed = (res.error_code != 0);

  return result;
}

}  // namespace val
}  // namespace tint
