// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/glsl/validate/validate.h"

#include <string>

#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "src/tint/utils/macros/static_init.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::glsl::validate {

namespace {

EShLanguage PipelineStageToEshLanguage(tint::ast::PipelineStage stage) {
    switch (stage) {
        case tint::ast::PipelineStage::kFragment:
            return EShLangFragment;
        case tint::ast::PipelineStage::kVertex:
            return EShLangVertex;
        case tint::ast::PipelineStage::kCompute:
            return EShLangCompute;
        default:
            TINT_UNREACHABLE();
            return EShLangVertex;
    }
}

}  // namespace

Result<SuccessType> Validate(const std::string& source, const EntryPointList& entry_points) {
    TINT_STATIC_INIT(glslang::InitializeProcess());

    for (auto entry_pt : entry_points) {
        EShLanguage lang = PipelineStageToEshLanguage(entry_pt.second);
        glslang::TShader shader(lang);
        const char* strings[1] = {source.c_str()};
        int lengths[1] = {static_cast<int>(source.length())};
        shader.setStringsWithLengths(strings, lengths, 1);
        shader.setEntryPoint("main");
        bool result =
            shader.parse(GetDefaultResources(), 310, EEsProfile, false, false, EShMsgDefault);
        if (!result) {
            StringStream err;
            err << "Error parsing GLSL shader:\n"
                << shader.getInfoLog() << "\n"
                << shader.getInfoDebugLog() << "\n";
            return Failure{err.str()};
        }
    }

    return Success;
}

}  // namespace tint::glsl::validate
