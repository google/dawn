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

// Package gen holds types and helpers for generating templated code from the
// intrinsic.def file.
//
// Used by tools/src/cmd/gen/main.go
package gen

import (
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/list"
	"dawn.googlesource.com/dawn/tools/src/lut"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

// IntrinsicTable holds data specific to the intrinsic_table.inl.tmpl template
type IntrinsicTable struct {
	// The semantic info
	Sem *sem.Sem

	// TMatchers are all the sem.TemplateType, sem.Type and sem.TypeMatchers.
	// These are all implemented by classes deriving from tint::TypeMatcher
	TMatchers     []sem.Named
	TMatcherIndex map[sem.Named]int // [object -> index] in TMatcher

	// NMatchers are all the sem.TemplateNumber and sem.EnumMatchers.
	// These are all implemented by classes deriving from tint::NumberMatcher
	NMatchers     []sem.Named
	NMatcherIndex map[sem.Named]int // [object -> index] in NMatchers

	MatcherIndices            []int            // kMatcherIndices table content
	TemplateTypes             []TemplateType   // kTemplateTypes table content
	TemplateNumbers           []TemplateNumber // kTemplateNumbers table content
	Parameters                []Parameter      // kParameters table content
	Overloads                 []Overload       // kOverloads table content
	Builtins                  []Intrinsic      // kBuiltins table content
	UnaryOperators            []Intrinsic      // kUnaryOperators table content
	BinaryOperators           []Intrinsic      // kBinaryOperators table content
	ConstructorsAndConverters []Intrinsic      // kInitializersAndConverters table content
}

// TemplateType is used to create the C++ TemplateTypeInfo structure
type TemplateType struct {
	// Name of the template type (e.g. 'T')
	Name string
	// Optional type matcher constraint.
	// Either an index in Matchers::type, or -1
	MatcherIndex int
}

// TemplateNumber is used to create the C++ TemplateNumberInfo structure
type TemplateNumber struct {
	// Name of the template number (e.g. 'N')
	Name string
	// Optional type matcher constraint.
	// Either an index in Matchers::type, or -1
	MatcherIndex int
}

// Parameter is used to create the C++ ParameterInfo structure
type Parameter struct {
	// The parameter usage (parameter name)
	Usage string

	// Index into IntrinsicTable.MatcherIndices, beginning the list of matchers
	// required to match the parameter type. The matcher indices index
	// into IntrinsicTable::TMatchers and / or IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	// The first index is always a TypeMatcher.
	MatcherIndicesOffset *int
}

// Overload is used to create the C++ OverloadInfo structure
type Overload struct {
	// Total number of parameters for the overload
	NumParameters int
	// Total number of template types for the overload
	NumTemplateTypes int
	// Total number of template numbers for the overload
	NumTemplateNumbers int
	// Index to the first template type in IntrinsicTable.TemplateTypes
	TemplateTypesOffset *int
	// Index to the first template number in IntrinsicTable.TemplateNumbers
	TemplateNumbersOffset *int
	// Index to the first parameter in IntrinsicTable.Parameters
	ParametersOffset *int
	// Index into IntrinsicTable.MatcherIndices, beginning the list of matchers
	// required to match the return type. The matcher indices index
	// into IntrinsicTable::TMatchers and / or IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	// The first index is always a TypeMatcher.
	ReturnMatcherIndicesOffset *int
	// StageUses describes the stages an overload can be used in
	CanBeUsedInStage sem.StageUses
	// True if the overload is marked as @must_use
	MustUse bool
	// True if the overload is marked as deprecated
	IsDeprecated bool
	// The kind of overload
	Kind string
	// The function name used to evaluate the overload at shader-creation time
	ConstEvalFunction string
}

// Intrinsic is used to create the C++ IntrinsicInfo structure
type Intrinsic struct {
	Name                 string
	OverloadDescriptions []string
	NumOverloads         int
	OverloadsOffset      *int
}

// Helper for building the IntrinsicTable
type IntrinsicTableBuilder struct {
	// The output of the builder
	IntrinsicTable

	// Lookup tables.
	// These are packed (compressed) once all the entries have been added.
	lut struct {
		matcherIndices  lut.LUT
		templateTypes   lut.LUT
		templateNumbers lut.LUT
		parameters      lut.LUT
		overloads       lut.LUT
	}
}

// Helper for building a single overload
type overloadBuilder struct {
	*IntrinsicTableBuilder
	// Maps TemplateParam to index in templateTypes
	templateTypeIndex map[sem.TemplateParam]int
	// Maps TemplateParam to index in templateNumbers
	templateNumberIndex map[sem.TemplateParam]int
	// Template types used by the overload
	templateTypes []TemplateType
	// Template numbers used by the overload
	templateNumbers []TemplateNumber
	// All parameters declared by the overload
	parameters []Parameter
	// Index into IntrinsicTable.MatcherIndices, beginning the list of matchers
	// required to match the return type. The matcher indices index
	// into IntrinsicTable::TMatchers and / or IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	// The first index is always a TypeMatcher.
	returnTypeMatcherIndicesOffset *int
}

// layoutMatchers assigns each of the TMatchers and NMatchers a unique index
// in the C++ Matchers::type and Matchers::number arrays, respectively.
func (b *IntrinsicTableBuilder) layoutMatchers(s *sem.Sem) {
	// First MaxTemplateTypes of TMatchers are template types
	b.TMatchers = make([]sem.Named, s.MaxTemplateTypes)
	for _, m := range s.Types {
		b.TMatcherIndex[m] = len(b.TMatchers)
		b.TMatchers = append(b.TMatchers, m)
	}
	for _, m := range s.TypeMatchers {
		b.TMatcherIndex[m] = len(b.TMatchers)
		b.TMatchers = append(b.TMatchers, m)
	}

	// First MaxTemplateNumbers of NMatchers are template numbers
	b.NMatchers = make([]sem.Named, s.MaxTemplateNumbers)
	for _, m := range s.EnumMatchers {
		b.NMatcherIndex[m] = len(b.NMatchers)
		b.NMatchers = append(b.NMatchers, m)
	}
}

// buildOverload constructs an Overload for a sem.Overload
func (b *IntrinsicTableBuilder) buildOverload(o *sem.Overload) (Overload, error) {
	ob := overloadBuilder{
		IntrinsicTableBuilder: b,
		templateTypeIndex:     map[sem.TemplateParam]int{},
		templateNumberIndex:   map[sem.TemplateParam]int{},
	}

	if err := ob.buildTemplateTypes(o); err != nil {
		return Overload{}, err
	}
	if err := ob.buildTemplateNumbers(o); err != nil {
		return Overload{}, err
	}
	if err := ob.buildParameters(o); err != nil {
		return Overload{}, err
	}
	if err := ob.buildReturnType(o); err != nil {
		return Overload{}, err
	}

	return Overload{
		NumParameters:              len(ob.parameters),
		NumTemplateTypes:           len(ob.templateTypes),
		NumTemplateNumbers:         len(ob.templateNumbers),
		TemplateTypesOffset:        b.lut.templateTypes.Add(ob.templateTypes),
		TemplateNumbersOffset:      b.lut.templateNumbers.Add(ob.templateNumbers),
		ParametersOffset:           b.lut.parameters.Add(ob.parameters),
		ReturnMatcherIndicesOffset: ob.returnTypeMatcherIndicesOffset,
		CanBeUsedInStage:           o.CanBeUsedInStage,
		MustUse:                    o.MustUse,
		IsDeprecated:               o.IsDeprecated,
		Kind:                       string(o.Decl.Kind),
		ConstEvalFunction:          o.ConstEvalFunction,
	}, nil
}

// buildTemplateTypes constructs the TemplateTypes used by the overload, populating
// b.templateTypes
func (b *overloadBuilder) buildTemplateTypes(o *sem.Overload) error {
	b.templateTypes = make([]TemplateType, len(o.TemplateTypes))
	for i, t := range o.TemplateTypes {
		b.templateTypeIndex[t] = i
		matcherIndex := -1
		if t.Type != nil {
			var err error
			matcherIndex, err = b.matcherIndex(t.Type)
			if err != nil {
				return err
			}
		}
		b.templateTypes[i] = TemplateType{
			Name:         t.Name,
			MatcherIndex: matcherIndex,
		}
	}
	return nil
}

// buildTemplateNumbers constructs the TemplateNumbers used by the overload, populating
// b.templateNumbers
func (b *overloadBuilder) buildTemplateNumbers(o *sem.Overload) error {
	b.templateNumbers = make([]TemplateNumber, len(o.TemplateNumbers))
	for i, t := range o.TemplateNumbers {
		b.templateNumberIndex[t] = i
		matcherIndex := -1
		if e, ok := t.(*sem.TemplateEnumParam); ok && e.Matcher != nil {
			var err error
			matcherIndex, err = b.matcherIndex(e.Matcher)
			if err != nil {
				return err
			}
		}
		b.templateNumbers[i] = TemplateNumber{
			Name:         t.GetName(),
			MatcherIndex: matcherIndex,
		}
	}
	return nil
}

// buildParameters constructs the Parameters used by the overload, populating
// b.parameters
func (b *overloadBuilder) buildParameters(o *sem.Overload) error {
	b.parameters = make([]Parameter, len(o.Parameters))
	for i, p := range o.Parameters {
		indices, err := b.collectMatcherIndices(p.Type)
		if err != nil {
			return err
		}

		b.parameters[i] = Parameter{
			Usage:                p.Name,
			MatcherIndicesOffset: b.lut.matcherIndices.Add(indices),
		}
	}
	return nil
}

// buildParameters calculates the matcher indices required to match the
// overload's return type (if the overload has a return value), possibly
// populating b.returnTypeMatcherIndicesOffset
func (b *overloadBuilder) buildReturnType(o *sem.Overload) error {
	if o.ReturnType != nil {
		indices, err := b.collectMatcherIndices(*o.ReturnType)
		if err != nil {
			return err
		}
		b.returnTypeMatcherIndicesOffset = b.lut.matcherIndices.Add(indices)
	}
	return nil
}

// matcherIndex returns the index of TMatcher or NMatcher in
// IntrinsicTable.TMatcher or IntrinsicTable.NMatcher, respectively.
func (b *overloadBuilder) matcherIndex(n sem.Named) (int, error) {
	switch n := n.(type) {
	case *sem.Type, *sem.TypeMatcher:
		if i, ok := b.TMatcherIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("matcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.TemplateTypeParam:
		if i, ok := b.templateTypeIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("templateTypeIndex missing entry for %v %T", n.Name, n)
	case *sem.EnumMatcher:
		if i, ok := b.NMatcherIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("matcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.TemplateEnumParam:
		if i, ok := b.templateNumberIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("templateNumberIndex missing entry for %v %T", n, n)
	case *sem.TemplateNumberParam:
		if i, ok := b.templateNumberIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("templateNumberIndex missing entry for %v %T", n, n)
	default:
		return 0, fmt.Errorf("overload.matcherIndex() does not handle %v %T", n, n)
	}
}

// collectMatcherIndices returns the full list of matcher indices required to
// match the fully-qualified-name. For names that have do not have templated
// arguments, collectMatcherIndices() will return a single TMatcher index.
// For names that do have templated arguments, collectMatcherIndices() returns
// a list of type matcher indices, starting with the target of the fully
// qualified name, then followed by each of the template arguments from left to
// right. Note that template arguments may themselves have template arguments,
// and so collectMatcherIndices() may call itself.
// The order of returned matcher indices is always the order of the fully
// qualified name as read from left to right.
// For example, calling collectMatcherIndices() for the fully qualified name:
//
//	A<B<C, D>, E<F, G<H>, I>
//
// Would return the matcher indices:
//
//	A, B, C, D, E, F, G, H, I
func (b *overloadBuilder) collectMatcherIndices(fqn sem.FullyQualifiedName) ([]int, error) {
	idx, err := b.matcherIndex(fqn.Target)
	if err != nil {
		return nil, err
	}
	out := []int{idx}
	for _, arg := range fqn.TemplateArguments {
		indices, err := b.collectMatcherIndices(arg.(sem.FullyQualifiedName))
		if err != nil {
			return nil, err
		}
		out = append(out, indices...)
	}
	return out, nil
}

// BuildIntrinsicTable builds the IntrinsicTable from the semantic info
func BuildIntrinsicTable(s *sem.Sem) (*IntrinsicTable, error) {
	b := IntrinsicTableBuilder{
		IntrinsicTable: IntrinsicTable{
			Sem:           s,
			TMatcherIndex: map[sem.Named]int{},
			NMatcherIndex: map[sem.Named]int{},
		},
	}
	b.lut.matcherIndices = lut.New(list.Wrap(&b.MatcherIndices))
	b.lut.templateTypes = lut.New(list.Wrap(&b.TemplateTypes))
	b.lut.templateNumbers = lut.New(list.Wrap(&b.TemplateNumbers))
	b.lut.parameters = lut.New(list.Wrap(&b.Parameters))
	b.lut.overloads = lut.New(list.Wrap(&b.Overloads))

	b.layoutMatchers(s)

	for _, intrinsics := range []struct {
		in  []*sem.Intrinsic
		out *[]Intrinsic
	}{
		{s.Builtins, &b.Builtins},
		{s.UnaryOperators, &b.UnaryOperators},
		{s.BinaryOperators, &b.BinaryOperators},
		{s.ConstructorsAndConverters, &b.ConstructorsAndConverters},
	} {
		out := make([]Intrinsic, len(intrinsics.in))
		for i, f := range intrinsics.in {
			overloads := make([]Overload, len(f.Overloads))
			overloadDescriptions := make([]string, len(f.Overloads))
			for i, o := range f.Overloads {
				overloadDescriptions[i] = fmt.Sprint(o.Decl)
				var err error
				if overloads[i], err = b.buildOverload(o); err != nil {
					return nil, err
				}
			}
			out[i] = Intrinsic{
				Name:                 f.Name,
				OverloadDescriptions: overloadDescriptions,
				NumOverloads:         len(overloads),
				OverloadsOffset:      b.lut.overloads.Add(overloads),
			}
		}
		*intrinsics.out = out
	}

	b.lut.matcherIndices.Compact()
	b.lut.templateTypes.Compact()
	b.lut.templateNumbers.Compact()
	b.lut.parameters.Compact()
	b.lut.overloads.Compact()

	return &b.IntrinsicTable, nil
}

// SplitDisplayName splits displayName into parts, where text wrapped in {}
// braces are not quoted and the rest is quoted. This is used to help process
// the string value of the [[display()]] decoration. For example:
//
//	SplitDisplayName("vec{N}<{T}>")
//
// would return the strings:
//
//	[`"vec"`, `N`, `"<"`, `T`, `">"`]
func SplitDisplayName(displayName string) []string {
	parts := []string{}
	pending := strings.Builder{}
	for _, r := range displayName {
		switch r {
		case '{':
			if pending.Len() > 0 {
				parts = append(parts, fmt.Sprintf(`"%v"`, pending.String()))
				pending.Reset()
			}
		case '}':
			if pending.Len() > 0 {
				parts = append(parts, pending.String())
				pending.Reset()
			}
		default:
			pending.WriteRune(r)
		}
	}
	if pending.Len() > 0 {
		parts = append(parts, fmt.Sprintf(`"%v"`, pending.String()))
	}
	return parts
}

// ElementType returns the nested type for type represented by the fully qualified name.
// If the type is not a composite type, then the fully qualified name is returned
func ElementType(fqn sem.FullyQualifiedName) sem.FullyQualifiedName {
	switch fqn.Target.GetName() {
	case "vec2", "vec3", "vec4":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	case "vec":
		return fqn.TemplateArguments[1].(sem.FullyQualifiedName)
	case "mat":
		return fqn.TemplateArguments[2].(sem.FullyQualifiedName)
	case "array":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	}
	return fqn
}

// DeepestElementType returns the inner most nested type for type represented by the
// fully qualified name.
func DeepestElementType(fqn sem.FullyQualifiedName) sem.FullyQualifiedName {
	switch fqn.Target.GetName() {
	case "vec2", "vec3", "vec4":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	case "vec":
		return fqn.TemplateArguments[1].(sem.FullyQualifiedName)
	case "mat":
		return DeepestElementType(fqn.TemplateArguments[2].(sem.FullyQualifiedName))
	case "array":
		return DeepestElementType(fqn.TemplateArguments[0].(sem.FullyQualifiedName))
	case "ptr":
		return DeepestElementType(fqn.TemplateArguments[1].(sem.FullyQualifiedName))
	}
	return fqn
}

// IsAbstract returns true if the FullyQualifiedName refers to an abstract numeric type float.
// Use DeepestElementType if you want to include vector, matrices and arrays of abstract types.
func IsAbstract(fqn sem.FullyQualifiedName) bool {
	switch fqn.Target.GetName() {
	case "ia", "fa":
		return true
	}
	return false
}

// IsDeclarable returns false if the FullyQualifiedName refers to an abstract
// numeric type, or if it starts with a leading underscore.
func IsDeclarable(fqn sem.FullyQualifiedName) bool {
	return !IsAbstract(DeepestElementType(fqn)) && !strings.HasPrefix(fqn.Target.GetName(), "_")
}

// OverloadUsesF16 returns true if the overload uses the f16 type anywhere in the signature.
func OverloadUsesF16(overload sem.Overload) bool {
	for _, param := range overload.Parameters {
		if DeepestElementType(param.Type).Target.GetName() == "f16" {
			return true
		}
	}
	if ret := overload.ReturnType; ret != nil {
		if DeepestElementType(*overload.ReturnType).Target.GetName() == "f16" {
			return true
		}
	}
	return false
}
