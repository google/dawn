// Copyright 2021 The Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::spirv::reader::test {

// Default to not dumping the SPIR-V assembly.
bool ASTParserWrapperForTest::dump_successfully_converted_spirv_ = false;

ASTParserWrapperForTest::ASTParserWrapperForTest(const std::vector<uint32_t>& input)
    : impl_(input) {}

ASTParserWrapperForTest::~ASTParserWrapperForTest() {
    if (dump_successfully_converted_spirv_ && !skip_dumping_spirv_ && !impl_.spv_binary().empty() &&
        impl_.success()) {
        std::string disassembly = Disassemble(impl_.spv_binary());
        std::cout << "BEGIN ConvertedOk:\n" << disassembly << "\nEND ConvertedOk" << std::endl;
    }
}

std::string ToString(const Program& program) {
    wgsl::writer::ASTPrinter writer(&program);
    writer.Generate();

    if (!writer.Diagnostics().empty()) {
        return "WGSL writer error: " + writer.Diagnostics().str();
    }
    return writer.Result();
}

std::string ToString(const Program& program, VectorRef<const ast::Statement*> stmts) {
    wgsl::writer::ASTPrinter writer(&program);
    for (const auto* stmt : stmts) {
        writer.EmitStatement(stmt);
    }
    if (!writer.Diagnostics().empty()) {
        return "WGSL writer error: " + writer.Diagnostics().str();
    }
    return writer.Result();
}

std::string ToString(const Program& program, const ast::Node* node) {
    wgsl::writer::ASTPrinter writer(&program);
    return Switch(
        node,
        [&](const ast::Expression* expr) {
            StringStream out;
            writer.EmitExpression(out, expr);
            if (!writer.Diagnostics().empty()) {
                return "WGSL writer error: " + writer.Diagnostics().str();
            }
            return out.str();
        },
        [&](const ast::Statement* stmt) {
            writer.EmitStatement(stmt);
            if (!writer.Diagnostics().empty()) {
                return "WGSL writer error: " + writer.Diagnostics().str();
            }
            return writer.Result();
        },
        [&](const ast::Identifier* ident) { return ident->symbol.Name(); },
        [&](Default) {
            return "<unhandled AST node type " + std::string(node->TypeInfo().name) + ">";
        });
}

}  // namespace tint::spirv::reader::test
