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

// Package parser provides a basic parser for the Tint builtin definition
// language
package parser

import (
	"fmt"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/ast"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/lexer"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/tok"
)

// Parse produces a list of tokens for the given source code
func Parse(source, filepath string) (*ast.AST, error) {
	runes := []rune(source)
	tokens, err := lexer.Lex(runes, filepath)
	if err != nil {
		return nil, err
	}

	p := parser{tokens: tokens}
	return p.parse()
}

type parser struct {
	tokens []tok.Token
	err    error
}

func (p *parser) parse() (*ast.AST, error) {
	out := ast.AST{}
	var decorations ast.Decorations
	for p.err == nil {
		t := p.peek(0)
		if t == nil {
			break
		}
		switch t.Kind {
		case tok.Ldeco:
			decorations = append(decorations, p.decorations()...)
		case tok.Enum:
			if len(decorations) > 0 {
				p.err = fmt.Errorf("%v unexpected decoration", decorations[0].Source)
			}
			out.Enums = append(out.Enums, p.enumDecl())
		case tok.Match:
			if len(decorations) > 0 {
				p.err = fmt.Errorf("%v unexpected decoration", decorations[0].Source)
			}
			out.Matchers = append(out.Matchers, p.matcherDecl())
		case tok.Type:
			out.Types = append(out.Types, p.typeDecl(decorations))
			decorations = nil
		case tok.Function:
			out.Functions = append(out.Functions, p.functionDecl(decorations))
			decorations = nil
		default:
			p.err = fmt.Errorf("%v unexpected token '%v'", t.Source, t.Kind)
		}
		if p.err != nil {
			return nil, p.err
		}
	}
	return &out, nil
}

func (p *parser) enumDecl() ast.EnumDecl {
	p.expect(tok.Enum, "enum declaration")
	name := p.expect(tok.Identifier, "enum name")
	e := ast.EnumDecl{Source: name.Source, Name: string(name.Runes)}
	p.expect(tok.Lbrace, "enum declaration")
	for p.err == nil && p.match(tok.Rbrace) == nil {
		e.Entries = append(e.Entries, p.enumEntry())
	}
	return e
}

func (p *parser) enumEntry() ast.EnumEntry {
	decos := p.decorations()
	name := p.expect(tok.Identifier, "enum entry")
	return ast.EnumEntry{Source: name.Source, Decorations: decos, Name: string(name.Runes)}
}

func (p *parser) matcherDecl() ast.MatcherDecl {
	p.expect(tok.Match, "matcher declaration")
	name := p.expect(tok.Identifier, "matcher name")
	m := ast.MatcherDecl{Source: name.Source, Name: string(name.Runes)}
	p.expect(tok.Colon, "matcher declaration")
	for p.err == nil {
		m.Options = append(m.Options, p.templatedName())
		if p.match(tok.Or) == nil {
			break
		}
	}
	return m
}

func (p *parser) typeDecl(decos ast.Decorations) ast.TypeDecl {
	p.expect(tok.Type, "type declaration")
	name := p.expect(tok.Identifier, "type name")
	m := ast.TypeDecl{
		Source:      name.Source,
		Decorations: decos,
		Name:        string(name.Runes),
	}
	if p.peekIs(0, tok.Lt) {
		m.TemplateParams = p.templateParams()
	}
	return m
}

func (p *parser) decorations() ast.Decorations {
	if p.match(tok.Ldeco) == nil {
		return nil
	}
	out := ast.Decorations{}
	for p.err == nil {
		name := p.expect(tok.Identifier, "decoration name")
		values := []string{}
		if p.match(tok.Lparen) != nil {
			for p.err == nil {
				values = append(values, p.string())
				if p.match(tok.Comma) == nil {
					break
				}
			}
			p.expect(tok.Rparen, "decoration values")
		}
		out = append(out, ast.Decoration{
			Source: name.Source,
			Name:   string(name.Runes),
			Values: values,
		})
		if !p.peekIs(0, tok.Comma) {
			break
		}
	}
	p.expect(tok.Rdeco, "decoration list")
	return out
}

func (p *parser) functionDecl(decos ast.Decorations) ast.FunctionDecl {
	p.expect(tok.Function, "function declaration")
	name := p.expect(tok.Identifier, "function name")
	f := ast.FunctionDecl{
		Source:      name.Source,
		Decorations: decos,
		Name:        string(name.Runes),
	}
	if p.peekIs(0, tok.Lt) {
		f.TemplateParams = p.templateParams()
	}
	f.Parameters = p.parameters()
	if p.match(tok.Arrow) != nil {
		ret := p.templatedName()
		f.ReturnType = &ret
	}
	return f
}

func (p *parser) parameters() ast.Parameters {
	l := ast.Parameters{}
	p.expect(tok.Lparen, "function parameter list")
	if p.match(tok.Rparen) == nil {
		for p.err == nil {
			l = append(l, p.parameter())
			if p.match(tok.Comma) == nil {
				break
			}
		}
		p.expect(tok.Rparen, "function parameter list")
	}
	return l
}

func (p *parser) parameter() ast.Parameter {
	if p.peekIs(1, tok.Colon) {
		// name type
		name := p.expect(tok.Identifier, "parameter name")
		p.expect(tok.Colon, "parameter type")
		return ast.Parameter{
			Source: name.Source,
			Name:   string(name.Runes),
			Type:   p.templatedName(),
		}
	}
	// type
	ty := p.templatedName()
	return ast.Parameter{
		Source: ty.Source,
		Type:   ty,
	}
}

func (p *parser) string() string {
	s := p.expect(tok.String, "string")
	return string(s.Runes)
}

func (p *parser) templatedName() ast.TemplatedName {
	name := p.expect(tok.Identifier, "type name")
	m := ast.TemplatedName{Source: name.Source, Name: string(name.Runes)}
	if p.match(tok.Lt) != nil {
		for p.err == nil {
			m.TemplateArgs = append(m.TemplateArgs, p.templatedName())
			if p.match(tok.Comma) == nil {
				break
			}
		}
		p.expect(tok.Gt, "template argument type list")
	}
	return m
}

func (p *parser) templateParams() ast.TemplateParams {
	t := ast.TemplateParams{}
	p.expect(tok.Lt, "template parameter list")
	for p.err == nil && p.peekIs(0, tok.Identifier) {
		t = append(t, p.templateParam())
	}
	p.expect(tok.Gt, "template parameter list")
	return t
}

func (p *parser) templateParam() ast.TemplateParam {
	name := p.match(tok.Identifier)
	t := ast.TemplateParam{
		Source: name.Source,
		Name:   string(name.Runes),
	}
	if p.match(tok.Colon) != nil {
		t.Type = p.templatedName()
	}
	p.match(tok.Comma)
	return t
}

func (p *parser) expect(kind tok.Kind, use string) tok.Token {
	if p.err != nil {
		return tok.Invalid
	}
	t := p.match(kind)
	if t == nil {
		if len(p.tokens) > 0 {
			p.err = fmt.Errorf("%v expected '%v' for %v, got '%v'",
				p.tokens[0].Source, kind, use, p.tokens[0].Kind)
		} else {
			p.err = fmt.Errorf("expected '%v' for %v, but reached end of file", kind, use)
		}
		return tok.Invalid
	}
	return *t
}

func (p *parser) ident(use string) string {
	return string(p.expect(tok.Identifier, use).Runes)
}

// TODO(bclayton): Currently unused, but will be needed for integer bounds
// func (p *parser) integer(use string) int {
// 	t := p.expect(tok.Integer, use)
// 	if t.Kind != tok.Integer {
// 		return 0
// 	}
// 	i, err := strconv.Atoi(string(t.Runes))
// 	if err != nil {
// 		p.err = err
// 		return 0
// 	}
// 	return i
// }

func (p *parser) match(kind tok.Kind) *tok.Token {
	if p.err != nil || len(p.tokens) == 0 {
		return nil
	}
	t := p.tokens[0]
	if t.Kind != kind {
		return nil
	}
	p.tokens = p.tokens[1:]
	return &t
}

func (p *parser) peekIs(i int, kind tok.Kind) bool {
	t := p.peek(i)
	if t == nil {
		return false
	}
	return t.Kind == kind
}

func (p *parser) peek(i int) *tok.Token {
	if len(p.tokens) <= i {
		return nil
	}
	return &p.tokens[i]
}
