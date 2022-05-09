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

// Package ast defines AST nodes that are produced by the Tint intrinsic
// definition parser
package ast

import (
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/tok"
)

// AST is the parsed syntax tree of the intrinsic definition file
type AST struct {
	Enums     []EnumDecl
	Types     []TypeDecl
	Matchers  []MatcherDecl
	Builtins  []IntrinsicDecl
	Operators []IntrinsicDecl
}

func (a AST) String() string {
	sb := strings.Builder{}
	for _, e := range a.Enums {
		fmt.Fprintf(&sb, "%v", e)
		fmt.Fprintln(&sb)
	}
	for _, p := range a.Types {
		fmt.Fprintf(&sb, "%v", p)
		fmt.Fprintln(&sb)
	}
	for _, m := range a.Matchers {
		fmt.Fprintf(&sb, "%v", m)
		fmt.Fprintln(&sb)
	}
	for _, b := range a.Builtins {
		fmt.Fprintf(&sb, "%v", b)
		fmt.Fprintln(&sb)
	}
	for _, o := range a.Operators {
		fmt.Fprintf(&sb, "%v", o)
		fmt.Fprintln(&sb)
	}
	return sb.String()
}

// EnumDecl describes an enumerator
type EnumDecl struct {
	Source  tok.Source
	Name    string
	Entries []EnumEntry
}

// Format implements the fmt.Formatter interface
func (e EnumDecl) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "enum %v {\n", e.Name)
	for _, e := range e.Entries {
		fmt.Fprintf(w, "  %v\n", e)
	}
	fmt.Fprintf(w, "}\n")
}

// EnumEntry describes an entry in a enumerator
type EnumEntry struct {
	Source      tok.Source
	Name        string
	Decorations Decorations
}

// Format implements the fmt.Formatter interface
func (e EnumEntry) Format(w fmt.State, verb rune) {
	if len(e.Decorations) > 0 {
		fmt.Fprintf(w, "%v %v", e.Decorations, e.Name)
	} else {
		fmt.Fprint(w, e.Name)
	}
}

// MatcherDecl describes a matcher declaration
type MatcherDecl struct {
	Source  tok.Source
	Name    string
	Options MatcherOptions
}

// Format implements the fmt.Formatter interface
func (m MatcherDecl) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "match %v", m.Name)
	fmt.Fprintf(w, ": ")
	m.Options.Format(w, verb)
}

// IntrinsicKind is either a Builtin or Operator
type IntrinsicKind string

const (
	Builtin  IntrinsicKind = "builtin"
	Operator IntrinsicKind = "operator"
)

// IntrinsicDecl describes a builtin or operator declaration
type IntrinsicDecl struct {
	Source         tok.Source
	Kind           IntrinsicKind
	Name           string
	Decorations    Decorations
	TemplateParams TemplateParams
	Parameters     Parameters
	ReturnType     *TemplatedName
}

// Format implements the fmt.Formatter interface
func (i IntrinsicDecl) Format(w fmt.State, verb rune) {
	switch i.Kind {
	case Builtin:
		fmt.Fprintf(w, "fn ")
	case Operator:
		fmt.Fprintf(w, "op ")
	}
	fmt.Fprintf(w, "%v", i.Name)
	i.TemplateParams.Format(w, verb)
	i.Parameters.Format(w, verb)
	if i.ReturnType != nil {
		fmt.Fprintf(w, " -> ")
		i.ReturnType.Format(w, verb)
	}
}

// Parameters is a list of parameter
type Parameters []Parameter

// Format implements the fmt.Formatter interface
func (l Parameters) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "(")
	for i, p := range l {
		if i > 0 {
			fmt.Fprintf(w, ", ")
		}
		p.Format(w, verb)
	}
	fmt.Fprintf(w, ")")
}

// Parameter describes a single parameter of a function
type Parameter struct {
	Source tok.Source
	Name   string // Optional
	Type   TemplatedName
}

// Format implements the fmt.Formatter interface
func (p Parameter) Format(w fmt.State, verb rune) {
	if p.Name != "" {
		fmt.Fprintf(w, "%v: ", p.Name)
	}
	p.Type.Format(w, verb)
}

// MatcherOptions is a list of TemplatedName
type MatcherOptions TemplatedNames

// Format implements the fmt.Formatter interface
func (o MatcherOptions) Format(w fmt.State, verb rune) {
	for i, mo := range o {
		if i > 0 {
			fmt.Fprintf(w, " | ")
		}
		mo.Format(w, verb)
	}
}

// TemplatedNames is a list of TemplatedName
// Example:
//   a<b>, c<d, e>
type TemplatedNames []TemplatedName

// Format implements the fmt.Formatter interface
func (l TemplatedNames) Format(w fmt.State, verb rune) {
	for i, n := range l {
		if i > 0 {
			fmt.Fprintf(w, ", ")
		}
		n.Format(w, verb)
	}
}

// TemplatedName is an identifier with optional templated arguments
// Example:
//  vec<N, T>
type TemplatedName struct {
	Source       tok.Source
	Name         string
	TemplateArgs TemplatedNames
}

// Format implements the fmt.Formatter interface
func (t TemplatedName) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v", t.Name)
	if len(t.TemplateArgs) > 0 {
		fmt.Fprintf(w, "<")
		t.TemplateArgs.Format(w, verb)
		fmt.Fprintf(w, ">")
	}
}

// TypeDecl describes a type declaration
type TypeDecl struct {
	Source         tok.Source
	Decorations    Decorations
	Name           string
	TemplateParams TemplateParams
}

// Format implements the fmt.Formatter interface
func (p TypeDecl) Format(w fmt.State, verb rune) {
	if len(p.Decorations) > 0 {
		p.Decorations.Format(w, verb)
		fmt.Fprintf(w, " type %v", p.Name)
	}
	fmt.Fprintf(w, "type %v", p.Name)
	p.TemplateParams.Format(w, verb)
}

// TemplateParams is a list of TemplateParam
// Example:
//   <A, B : TyB>
type TemplateParams []TemplateParam

// Format implements the fmt.Formatter interface
func (p TemplateParams) Format(w fmt.State, verb rune) {
	if len(p) > 0 {
		fmt.Fprintf(w, "<")
		for i, tp := range p {
			if i > 0 {
				fmt.Fprintf(w, ", ")
			}
			tp.Format(w, verb)
		}
		fmt.Fprintf(w, ">")
	}
}

// TemplateParam describes a template parameter with optional type
// Example:
//   <Name>
//   <Name: Type>
type TemplateParam struct {
	Source tok.Source
	Name   string
	Type   TemplatedName // Optional
}

// Format implements the fmt.Formatter interface
func (t TemplateParam) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v", t.Name)
	if t.Type.Name != "" {
		fmt.Fprintf(w, " : ")
		t.Type.Format(w, verb)
	}
}

// Decorations is a list of Decoration
// Example:
//   [[a(x), b(y)]]
type Decorations []Decoration

// Format implements the fmt.Formatter interface
func (l Decorations) Format(w fmt.State, verb rune) {
	fmt.Fprint(w, "[[")
	for i, d := range l {
		if i > 0 {
			fmt.Fprintf(w, ", ")
		}
		d.Format(w, verb)
	}
	fmt.Fprint(w, "]]")
}

// Take looks up the decoration with the given name. If the decoration is found
// it is removed from the Decorations list and returned, otherwise nil is
// returned and the Decorations are not altered.
func (l *Decorations) Take(name string) *Decoration {
	for i, d := range *l {
		if d.Name == name {
			*l = append((*l)[:i], (*l)[i+1:]...)
			return &d
		}
	}
	return nil
}

// Decoration describes a single decoration
// Example:
//   a(x)
type Decoration struct {
	Source tok.Source
	Name   string
	Values []string
}

// Format implements the fmt.Formatter interface
func (d Decoration) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v", d.Name)
	if len(d.Values) > 0 {
		fmt.Fprintf(w, "(")
		for i, v := range d.Values {
			if i > 0 {
				fmt.Fprint(w, ", ")
			}
			fmt.Fprintf(w, "%v", v)
		}
		fmt.Fprintf(w, ")")
	}
}
