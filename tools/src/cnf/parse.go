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

package cnf

import (
	"fmt"
	"strings"
	"unicode"
)

// Parse parses the string, returning an expression
func Parse(s string) (Expr, error) {
	tokens, err := lex(s)
	if err != nil {
		return nil, err
	}
	if len(tokens) == 0 {
		return nil, nil
	}
	p := parser{string: s, tokens: tokens}
	return p.parse()
}

type tokenKind string

const (
	tokEOF    = "<end of expression>"
	tokLParen = "("
	tokRParen = ")"
	tokIdent  = "ident"
	tokAnd    = "&&"
	tokOr     = "||"
	tokNot    = "!"
)

type token struct {
	kind       tokenKind
	start, end int
}

type parser struct {
	string string
	tokens []token
}

func (p *parser) parse() (Expr, error) {
	return p.binary()
}

func (p *parser) binary() (Expr, error) {
	expr, err := p.unary()
	if err != nil {
		return nil, err
	}

	var delimiter tokenKind
	var op func(Expr, Expr) Expr

	switch p.peek().kind {
	case tokAnd:
		delimiter, op = tokAnd, And
	case tokOr:
		delimiter, op = tokOr, Or
	default:
		return expr, nil
	}

	for {
		switch p.peek().kind {
		case delimiter:
			p.next()
			rhs, err := p.unary()
			if err != nil {
				return nil, err
			}
			expr = op(expr, rhs)
		case tokRParen, tokEOF:
			return expr, nil
		case tokAnd, tokOr:
			t := p.next()
			return nil, p.err(t, "cannot mix '&&' and '||' without parentheses")
		default:
			t := p.next()
			return nil, p.err(t, "expected '%v'", delimiter)
		}
	}
}

func (p *parser) unary() (Expr, error) {
	if p.match(tokNot) {
		expr, err := p.unary()
		if err != nil {
			return nil, err
		}
		return Not(expr), nil
	}

	// '(' binary ')'
	if p.match(tokLParen) {
		expr, err := p.binary()
		if err != nil {
			return nil, err
		}
		if _, err := p.expect(tokRParen); err != nil {
			return nil, err
		}
		return expr, nil
	}

	return p.ident()
}

func (p *parser) ident() (Expr, error) {
	identTok, err := p.expect(tokIdent)
	if err != nil {
		return nil, err
	}
	ident := p.string[identTok.start:identTok.end]
	return Expr{{{Var: ident}}}, nil
}

func (p *parser) expect(t tokenKind) (token, error) {
	got := p.next()
	if t != got.kind {
		if got.kind == tokEOF {
			return token{}, p.err(got, "expected '%v'", t)
		}
		return token{}, p.err(got, "expected '%v', got '%v'", t, p.string[got.start:got.end])
	}
	return got, nil
}

func (p *parser) peek() token {
	if len(p.tokens) == 0 {
		return token{kind: tokEOF, start: len(p.string), end: len(p.string)}
	}
	return p.tokens[0]
}

func (p *parser) match(t tokenKind) bool {
	if p.peek().kind == t {
		p.next()
		return true
	}
	return false
}

func (p *parser) next() token {
	t := p.peek()
	if len(p.tokens) > 0 {
		p.tokens = p.tokens[1:]
	}
	return t
}

func (p *parser) err(t token, msg string, args ...any) error {
	if t.end < t.start {
		panic("token end is before start")
	}
	return ParseErr{
		Start:      t.start,
		End:        t.end,
		Expression: p.string,
		Message:    fmt.Sprintf(msg, args...),
	}
}

type ParseErr struct {
	Start, End int
	Expression string
	Message    string
}

func (e ParseErr) Error() string {
	n := e.End - e.Start
	if n == 0 {
		n = 1
	}
	return fmt.Sprintf("Parse error:\n\n%v\n%v%v\n\n%v",
		e.Expression,
		strings.Repeat(" ", e.Start), strings.Repeat("^", n),
		e.Message)
}

func lex(in string) ([]token, error) {
	runes := []rune(in)
	offset := 0

	peek := func(i int) rune {
		if offset+i < len(runes) {
			return runes[offset+i]
		}
		return 0
	}

	advance := func(predicate func(r rune) bool) (start, end int) {
		start = offset
		for i, r := range runes[offset:] {
			if !predicate(r) {
				offset += i
				return start, offset
			}
		}
		offset = len(runes)
		return start, offset
	}

	err := func(msg string, args ...any) ParseErr {
		return ParseErr{
			Start:      offset,
			End:        offset + 1,
			Expression: in,
			Message:    fmt.Sprintf(msg, args...),
		}
	}

	tokens := []token{}
	for offset < len(runes) {
		advance(unicode.IsSpace)

		p0 := peek(0)
		switch {
		case isIdentifier(p0):
			start, end := advance(isIdentifier)
			tokens = append(tokens, token{kind: tokIdent, start: start, end: end})
		case p0 == '!':
			tokens = append(tokens, token{kind: tokNot, start: offset, end: offset + 1})
			offset++
		case p0 == '(':
			tokens = append(tokens, token{kind: tokLParen, start: offset, end: offset + 1})
			offset++
		case p0 == ')':
			tokens = append(tokens, token{kind: tokRParen, start: offset, end: offset + 1})
			offset++
		default:
			p1 := peek(1)
			switch {
			case p0 == '&' && p1 == '&':
				tokens = append(tokens, token{kind: tokAnd, start: offset, end: offset + 2})
				offset += 2
			case p0 == '|' && p1 == '|':
				tokens = append(tokens, token{kind: tokOr, start: offset, end: offset + 2})
				offset += 2
			default:
				return nil, err("unexpected character '%c'", p0)
			}
		}
	}
	return tokens, nil
}

func isIdentifier(r rune) bool {
	return unicode.IsLetter(r) || r == '_'
}
