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

#include "src/tint/reader/wgsl/classify_template_args.h"

#include <vector>

#include "src/tint/debug.h"
#include "src/tint/utils/vector.h"

namespace tint::reader::wgsl {

namespace {

/// If the token at index @p idx is a '>>', '>=' or '>>=', then the token is split into two, with
/// the first being '>', otherwise MaybeSplit() will be a no-op.
/// @param tokens the vector of tokens
/// @param idx the index of the token to (maybe) split
void MaybeSplit(std::vector<Token>& tokens, size_t idx) {
    Token* token = &tokens[idx];
    switch (token->type()) {
        case Token::Type::kShiftRight:  //  '>>'
            TINT_ASSERT(Reader, token[1].type() == Token::Type::kPlaceholder);
            token[0].SetType(Token::Type::kGreaterThan);
            token[1].SetType(Token::Type::kGreaterThan);
            break;
        case Token::Type::kGreaterThanEqual:  //  '>='
            TINT_ASSERT(Reader, token[1].type() == Token::Type::kPlaceholder);
            token[0].SetType(Token::Type::kGreaterThan);
            token[1].SetType(Token::Type::kEqual);
            break;
        case Token::Type::kShiftRightEqual:  // '>>='
            TINT_ASSERT(Reader, token[1].type() == Token::Type::kPlaceholder);
            token[0].SetType(Token::Type::kGreaterThan);
            token[1].SetType(Token::Type::kGreaterThanEqual);
            break;
        default:
            break;
    }
}

}  // namespace

void ClassifyTemplateArguments(std::vector<Token>& tokens) {
    const size_t count = tokens.size();

    // The current expression nesting depth.
    // Each '(', '[' increments the depth.
    // Each ')', ']' decrements the depth.
    uint64_t expr_depth = 0;

    // A stack of '<' tokens.
    // Used to pair '<' and '>' tokens at the same expression depth.
    struct StackEntry {
        Token* token;         // A pointer to the opening '<' token
        uint64_t expr_depth;  // The value of 'expr_depth' for the opening '<'
    };
    utils::Vector<StackEntry, 16> stack;

    for (size_t i = 0; i < count - 1; i++) {
        switch (tokens[i].type()) {
            case Token::Type::kIdentifier:
            case Token::Type::kVar:
            case Token::Type::kBitcast: {
                auto& next = tokens[i + 1];
                if (next.type() == Token::Type::kLessThan) {
                    // ident '<'
                    // Push this '<' to the stack, along with the current nesting expr_depth.
                    stack.Push(StackEntry{&tokens[i + 1], expr_depth});
                    i++;  // Skip the '<'
                }
                break;
            }
            case Token::Type::kGreaterThan:       // '>'
            case Token::Type::kShiftRight:        // '>>'
            case Token::Type::kGreaterThanEqual:  // '>='
            case Token::Type::kShiftRightEqual:   // '>>='
                if (!stack.IsEmpty() && stack.Back().expr_depth == expr_depth) {
                    // '<' and '>' at same expr_depth, and no terminating tokens in-between.
                    // Consider both as a template argument list.
                    MaybeSplit(tokens, i);
                    stack.Pop().token->SetType(Token::Type::kTemplateArgsLeft);
                    tokens[i].SetType(Token::Type::kTemplateArgsRight);
                }
                break;

            case Token::Type::kParenLeft:    // '('
            case Token::Type::kBracketLeft:  // '['
                // Entering a nested expression
                expr_depth++;
                break;

            case Token::Type::kParenRight:    // ')'
            case Token::Type::kBracketRight:  // ']'
                // Exiting a nested expression
                // Pop the stack until we return to the current expression expr_depth
                while (!stack.IsEmpty() && stack.Back().expr_depth == expr_depth) {
                    stack.Pop();
                }
                if (expr_depth > 0) {
                    expr_depth--;
                }
                break;

            case Token::Type::kSemicolon:  // ';'
            case Token::Type::kBraceLeft:  // '{'
            case Token::Type::kEqual:      // '='
            case Token::Type::kColon:      // ':'
                // Expression terminating tokens. No opening template list can hold these tokens, so
                // clear the stack and expression depth.
                expr_depth = 0;
                stack.Clear();
                break;

            case Token::Type::kOrOr:    // '||'
            case Token::Type::kAndAnd:  // '&&'
                // Treat 'a < b || c > d' as a logical binary operator of two comparison operators
                // instead of a single template argument 'b||c'.
                // Use parentheses around 'b||c' to parse as a template argument list.
                while (!stack.IsEmpty() && stack.Back().expr_depth == expr_depth) {
                    stack.Pop();
                }
                break;

            default:
                break;
        }
    }
}

}  // namespace tint::reader::wgsl
