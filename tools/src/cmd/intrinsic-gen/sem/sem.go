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
	"dawn.googlesource.com/tint/tools/src/cmd/intrinsic-gen/ast"
)

// Sem is the root of the semantic tree
type Sem struct {
	Enums        []*Enum
	Types        []*Type
	TypeMatchers []*TypeMatcher
	EnumMatchers []*EnumMatcher
	Functions    []*Function
	// Maximum number of open-types used across all intrinsics
	MaxOpenTypes int
	// Maximum number of open-numbers used across all intrinsics
	MaxOpenNumbers int
	// The alphabetically sorted list of unique parameter names
	UniqueParameterNames []string
}

// New returns a new Sem
func New() *Sem {
	return &Sem{
		Enums:        []*Enum{},
		Types:        []*Type{},
		TypeMatchers: []*TypeMatcher{},
		EnumMatchers: []*EnumMatcher{},
		Functions:    []*Function{},
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

// EnumEntry is an entry in an enumerator
type EnumEntry struct {
	Enum       *Enum
	Name       string
	IsInternal bool // True if this entry is not part of the WGSL grammar
}

// Type declares a type
type Type struct {
	TemplateParams []TemplateParam
	Decl           ast.TypeDecl
	Name           string
	DisplayName    string
}

// TypeMatcher declares a type matcher
type TypeMatcher struct {
	TemplateParams []TemplateParam
	Decl           ast.MatcherDecl
	Name           string
	Types          []*Type
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

// Function describes the overloads of an intrinsic function
type Function struct {
	Name      string
	Overloads []*Overload
}

// Overload describes a single overload of a function
type Overload struct {
	Decl           ast.FunctionDecl
	TemplateParams []TemplateParam
	OpenTypes      []*TemplateTypeParam
	OpenNumbers    []TemplateParam
	ReturnType     *FullyQualifiedName
	Parameters     []Parameter
}

// Parameter describes a single parameter of a function overload
type Parameter struct {
	Name string
	Type FullyQualifiedName
}

// FullyQualifiedName is the usage of a Type, TypeMatcher or TemplateTypeParam
type FullyQualifiedName struct {
	Target            Named
	TemplateArguments []FullyQualifiedName
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
