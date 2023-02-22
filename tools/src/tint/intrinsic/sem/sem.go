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

package sem

import (
	"fmt"
	"sort"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/ast"
)

// Sem is the root of the semantic tree
type Sem struct {
	Enums                     []*Enum
	Types                     []*Type
	TypeMatchers              []*TypeMatcher
	EnumMatchers              []*EnumMatcher
	Builtins                  []*Intrinsic
	UnaryOperators            []*Intrinsic
	BinaryOperators           []*Intrinsic
	ConstructorsAndConverters []*Intrinsic
	// Maximum number of template types used across all builtins
	MaxTemplateTypes int
	// Maximum number of template numbers used across all builtins
	MaxTemplateNumbers int
	// The alphabetically sorted list of unique parameter names
	UniqueParameterNames []string
}

// Enum returns the enum with the given name
func (s *Sem) Enum(name string) *Enum {
	for _, e := range s.Enums {
		if e.Name == name {
			return e
		}
	}
	return nil
}

// New returns a new Sem
func New() *Sem {
	return &Sem{
		Enums:           []*Enum{},
		Types:           []*Type{},
		TypeMatchers:    []*TypeMatcher{},
		EnumMatchers:    []*EnumMatcher{},
		Builtins:        []*Intrinsic{},
		UnaryOperators:  []*Intrinsic{},
		BinaryOperators: []*Intrinsic{},
	}
}

// Enum describes an enumerator
type Enum struct {
	Decl    ast.EnumDecl
	Name    string
	Entries []*EnumEntry
}

// FindEntry returns the enum entry with the given name
func (e *Enum) FindEntry(name string) *EnumEntry {
	for _, entry := range e.Entries {
		if entry.Name == name {
			return entry
		}
	}
	return nil
}

// PublicEntries returns the enum entries that are not annotated with @internal
func (e *Enum) PublicEntries() []*EnumEntry {
	out := make([]*EnumEntry, 0, len(e.Entries))
	for _, entry := range e.Entries {
		if !entry.IsInternal {
			out = append(out, entry)
		}
	}
	return out
}

// NameSet returns a set of all the enum entry names
func (e *Enum) NameSet() container.Set[string] {
	out := container.NewSet[string]()
	for _, entry := range e.Entries {
		out.Add(entry.Name)
	}
	return out
}

// EnumEntry is an entry in an enumerator
type EnumEntry struct {
	Enum       *Enum
	Name       string
	IsInternal bool // True if this entry is not part of the WGSL grammar
}

// Format implements the fmt.Formatter interface
func (e EnumEntry) Format(w fmt.State, verb rune) {
	if e.IsInternal {
		fmt.Fprint(w, "[[internal]] ")
	}
	fmt.Fprint(w, e.Name)
}

// Type declares a type
type Type struct {
	TemplateParams []TemplateParam
	Decl           ast.TypeDecl
	Name           string
	DisplayName    string
	Precedence     int
}

// TypeMatcher declares a type matcher
type TypeMatcher struct {
	TemplateParams []TemplateParam
	Decl           ast.MatcherDecl
	Name           string
	Types          []*Type
}

func (t TypeMatcher) PrecedenceSortedTypes() []*Type {
	out := make([]*Type, len(t.Types))
	copy(out, t.Types)
	sort.Slice(out, func(i, j int) bool { return out[i].Precedence > out[j].Precedence })
	return out
}

// EnumMatcher declares a enum matcher
type EnumMatcher struct {
	TemplateParams []TemplateParam
	Decl           ast.MatcherDecl
	Name           string
	Enum           *Enum
	Options        []*EnumEntry
}

// TemplateEnumParam is a template enum parameter
type TemplateEnumParam struct {
	Name    string
	Enum    *Enum
	Matcher *EnumMatcher // Optional
}

// TemplateTypeParam is a template type parameter
type TemplateTypeParam struct {
	Name string
	Type ResolvableType
}

// TemplateNumberParam is a template type parameter
type TemplateNumberParam struct {
	Name string
}

// Intrinsic describes the overloads of a builtin or operator
type Intrinsic struct {
	Name      string
	Overloads []*Overload
}

// Overload describes a single overload of a builtin or operator
type Overload struct {
	Decl              ast.IntrinsicDecl
	Intrinsic         *Intrinsic
	TemplateParams    []TemplateParam
	TemplateTypes     []*TemplateTypeParam
	TemplateNumbers   []TemplateParam
	ReturnType        *FullyQualifiedName
	Parameters        []Parameter
	CanBeUsedInStage  StageUses
	MustUse           bool   // True if function cannot be used as a statement
	IsDeprecated      bool   // True if this overload is deprecated
	ConstEvalFunction string // Name of the function used to evaluate the intrinsic at shader creation time
}

// StageUses describes the stages an overload can be used in
type StageUses struct {
	Vertex   bool
	Fragment bool
	Compute  bool
}

// List returns the stage uses as a string list
func (u StageUses) List() []string {
	out := []string{}
	if u.Vertex {
		out = append(out, "vertex")
	}
	if u.Fragment {
		out = append(out, "fragment")
	}
	if u.Compute {
		out = append(out, "compute")
	}
	return out
}

// Format implements the fmt.Formatter interface
func (o Overload) Format(w fmt.State, verb rune) {
	switch o.Decl.Kind {
	case ast.Builtin:
		fmt.Fprintf(w, "fn ")
	case ast.Operator:
		fmt.Fprintf(w, "op ")
	}
	fmt.Fprintf(w, "%v", o.Intrinsic.Name)
	if len(o.TemplateParams) > 0 {
		fmt.Fprintf(w, "<")
		for i, t := range o.TemplateParams {
			if i > 0 {
				fmt.Fprint(w, ", ")
			}
			fmt.Fprintf(w, "%v", t)
		}
		fmt.Fprintf(w, ">")
	}
	fmt.Fprint(w, "(")
	for i, p := range o.Parameters {
		if i > 0 {
			fmt.Fprint(w, ", ")
		}
		fmt.Fprintf(w, "%v", p)
	}
	fmt.Fprint(w, ")")
	if o.ReturnType != nil {
		fmt.Fprintf(w, " -> %v", o.ReturnType)
	}
}

// Parameter describes a single parameter of a function overload
type Parameter struct {
	Name      string
	Type      FullyQualifiedName
	IsConst   bool    // Did this parameter have a @const attribute?
	TestValue float64 // Value to use for end-to-end tests
}

// Format implements the fmt.Formatter interface
func (p Parameter) Format(w fmt.State, verb rune) {
	if p.IsConst {
		fmt.Fprint(w, "@const ")
	}
	if p.Name != "" {
		fmt.Fprintf(w, "%v: ", p.Name)
	}
	fmt.Fprintf(w, "%v", p.Type)
}

// FullyQualifiedName is the usage of a Type, TypeMatcher or TemplateTypeParam
type FullyQualifiedName struct {
	Target            Named
	TemplateArguments []interface{}
}

// Format implements the fmt.Formatter interface
func (f FullyQualifiedName) Format(w fmt.State, verb rune) {
	fmt.Fprint(w, f.Target.GetName())
	if len(f.TemplateArguments) > 0 {
		fmt.Fprintf(w, "<")
		for i, t := range f.TemplateArguments {
			if i > 0 {
				fmt.Fprint(w, ", ")
			}
			fmt.Fprintf(w, "%v", t)
		}
		fmt.Fprintf(w, ">")
	}
}

// TemplateParam is a TemplateEnumParam, TemplateTypeParam or TemplateNumberParam
type TemplateParam interface {
	Named
	isTemplateParam()
}

func (*TemplateEnumParam) isTemplateParam()   {}
func (*TemplateTypeParam) isTemplateParam()   {}
func (*TemplateNumberParam) isTemplateParam() {}

// ResolvableType is a Type, TypeMatcher or TemplateTypeParam
type ResolvableType interface {
	Named
	isResolvableType()
}

func (*Type) isResolvableType()              {}
func (*TypeMatcher) isResolvableType()       {}
func (*TemplateTypeParam) isResolvableType() {}

// Named is something that can be looked up by name
type Named interface {
	isNamed()
	GetName() string
}

func (*Enum) isNamed()                {}
func (*EnumEntry) isNamed()           {}
func (*Type) isNamed()                {}
func (*TypeMatcher) isNamed()         {}
func (*EnumMatcher) isNamed()         {}
func (*TemplateTypeParam) isNamed()   {}
func (*TemplateEnumParam) isNamed()   {}
func (*TemplateNumberParam) isNamed() {}

// GetName returns the name of the Enum
func (e *Enum) GetName() string { return e.Name }

// GetName returns the name of the EnumEntry
func (e *EnumEntry) GetName() string { return e.Name }

// GetName returns the name of the Type
func (t *Type) GetName() string { return t.Name }

// GetName returns the name of the TypeMatcher
func (t *TypeMatcher) GetName() string { return t.Name }

// GetName returns the name of the EnumMatcher
func (e *EnumMatcher) GetName() string { return e.Name }

// GetName returns the name of the TemplateTypeParam
func (t *TemplateTypeParam) GetName() string { return t.Name }

// GetName returns the name of the TemplateEnumParam
func (t *TemplateEnumParam) GetName() string { return t.Name }

// GetName returns the name of the TemplateNumberParam
func (t *TemplateNumberParam) GetName() string { return t.Name }
