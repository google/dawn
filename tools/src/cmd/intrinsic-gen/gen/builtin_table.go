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

package gen

import (
	"fmt"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/sem"
	"dawn.googlesource.com/dawn/tools/src/list"
	"dawn.googlesource.com/dawn/tools/src/lut"
)

// BuiltinTable holds data specific to the intrinsic_table.inl.tmpl template
type BuiltinTable struct {
	// The semantic info
	Sem *sem.Sem

	// TMatchers are all the sem.OpenType, sem.Type and sem.TypeMatchers.
	// These are all implemented by classes deriving from tint::TypeMatcher
	TMatchers     []sem.Named
	TMatcherIndex map[sem.Named]int // [object -> index] in TMatcher

	// NMatchers are all the sem.OpenNumber and sem.EnumMatchers.
	// These are all implemented by classes deriving from tint::NumberMatcher
	NMatchers     []sem.Named
	NMatcherIndex map[sem.Named]int // [object -> index] in NMatchers

	MatcherIndices []int        // kMatcherIndices table content
	OpenTypes      []OpenType   // kOpenTypes table content
	OpenNumbers    []OpenNumber // kOpenNumbers table content
	Parameters     []Parameter  // kParameters table content
	Overloads      []Overload   // kOverloads table content
	Functions      []Function   // kBuiltins table content
}

// OpenType is used to create the C++ OpenTypeInfo structure
type OpenType struct {
	// Name of the open type (e.g. 'T')
	Name string
	// Optional type matcher constraint.
	// Either an index in Matchers::type, or -1
	MatcherIndex int
}

// OpenNumber is used to create the C++ OpenNumberInfo structure
type OpenNumber struct {
	// Name of the open number (e.g. 'N')
	Name string
	// Optional type matcher constraint.
	// Either an index in Matchers::type, or -1
	MatcherIndex int
}

// Parameter is used to create the C++ ParameterInfo structure
type Parameter struct {
	// The parameter usage (parameter name)
	Usage string

	// Index into BuiltinTable.MatcherIndices, beginning the list of matchers
	// required to match the parameter type. The matcher indices index
	// into BuiltinTable::TMatchers and / or BuiltinTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	// The first index is always a TypeMatcher.
	MatcherIndicesOffset *int
}

// Overload is used to create the C++ OverloadInfo structure
type Overload struct {
	// Total number of parameters for the overload
	NumParameters int
	// Total number of open types for the overload
	NumOpenTypes int
	// Total number of open numbers for the overload
	NumOpenNumbers int
	// Index to the first open type in BuiltinTable.OpenTypes
	OpenTypesOffset *int
	// Index to the first open number in BuiltinTable.OpenNumbers
	OpenNumbersOffset *int
	// Index to the first parameter in BuiltinTable.Parameters
	ParametersOffset *int
	// Index into BuiltinTable.MatcherIndices, beginning the list of matchers
	// required to match the return type. The matcher indices index
	// into BuiltinTable::TMatchers and / or BuiltinTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	// The first index is always a TypeMatcher.
	ReturnMatcherIndicesOffset *int
	// StageUses describes the stages an overload can be used in
	CanBeUsedInStage sem.StageUses
	// True if the overload is marked as deprecated
	IsDeprecated bool
}

// Function is used to create the C++ IntrinsicInfo structure
type Function struct {
	OverloadDescriptions []string
	NumOverloads         int
	OverloadsOffset      *int
}

// Helper for building the BuiltinTable
type BuiltinTableBuilder struct {
	// The output of the builder
	BuiltinTable

	// Lookup tables.
	// These are packed (compressed) once all the entries have been added.
	lut struct {
		matcherIndices lut.LUT
		openTypes      lut.LUT
		openNumbers    lut.LUT
		parameters     lut.LUT
		overloads      lut.LUT
	}
}

// Helper for building a single overload
type overloadBuilder struct {
	*BuiltinTableBuilder
	// Maps TemplateParam to index in openTypes
	openTypeIndex map[sem.TemplateParam]int
	// Maps TemplateParam to index in openNumbers
	openNumberIndex map[sem.TemplateParam]int
	// Open types used by the overload
	openTypes []OpenType
	// Open numbers used by the overload
	openNumbers []OpenNumber
	// All parameters declared by the overload
	parameters []Parameter
	// Index into BuiltinTable.MatcherIndices, beginning the list of matchers
	// required to match the return type. The matcher indices index
	// into BuiltinTable::TMatchers and / or BuiltinTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	// The first index is always a TypeMatcher.
	returnTypeMatcherIndicesOffset *int
}

// layoutMatchers assigns each of the TMatchers and NMatchers a unique index
// in the C++ Matchers::type and Matchers::number arrays, respectively.
func (b *BuiltinTableBuilder) layoutMatchers(s *sem.Sem) {
	// First MaxOpenTypes of TMatchers are open types
	b.TMatchers = make([]sem.Named, s.MaxOpenTypes)
	for _, m := range s.Types {
		b.TMatcherIndex[m] = len(b.TMatchers)
		b.TMatchers = append(b.TMatchers, m)
	}
	for _, m := range s.TypeMatchers {
		b.TMatcherIndex[m] = len(b.TMatchers)
		b.TMatchers = append(b.TMatchers, m)
	}

	// First MaxOpenNumbers of NMatchers are open numbers
	b.NMatchers = make([]sem.Named, s.MaxOpenNumbers)
	for _, m := range s.EnumMatchers {
		b.NMatcherIndex[m] = len(b.NMatchers)
		b.NMatchers = append(b.NMatchers, m)
	}
}

// buildOverload constructs an Overload for a sem.Overload
func (b *BuiltinTableBuilder) buildOverload(o *sem.Overload) (Overload, error) {
	ob := overloadBuilder{
		BuiltinTableBuilder: b,
		openTypeIndex:       map[sem.TemplateParam]int{},
		openNumberIndex:     map[sem.TemplateParam]int{},
	}

	if err := ob.buildOpenTypes(o); err != nil {
		return Overload{}, err
	}
	if err := ob.buildOpenNumbers(o); err != nil {
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
		NumOpenTypes:               len(ob.openTypes),
		NumOpenNumbers:             len(ob.openNumbers),
		OpenTypesOffset:            b.lut.openTypes.Add(ob.openTypes),
		OpenNumbersOffset:          b.lut.openNumbers.Add(ob.openNumbers),
		ParametersOffset:           b.lut.parameters.Add(ob.parameters),
		ReturnMatcherIndicesOffset: ob.returnTypeMatcherIndicesOffset,
		CanBeUsedInStage:           o.CanBeUsedInStage,
		IsDeprecated:               o.IsDeprecated,
	}, nil
}

// buildOpenTypes constructs the OpenTypes used by the overload, populating
// b.openTypes
func (b *overloadBuilder) buildOpenTypes(o *sem.Overload) error {
	b.openTypes = make([]OpenType, len(o.OpenTypes))
	for i, t := range o.OpenTypes {
		b.openTypeIndex[t] = i
		matcherIndex := -1
		if t.Type != nil {
			var err error
			matcherIndex, err = b.matcherIndex(t.Type)
			if err != nil {
				return err
			}
		}
		b.openTypes[i] = OpenType{
			Name:         t.Name,
			MatcherIndex: matcherIndex,
		}
	}
	return nil
}

// buildOpenNumbers constructs the OpenNumbers used by the overload, populating
// b.openNumbers
func (b *overloadBuilder) buildOpenNumbers(o *sem.Overload) error {
	b.openNumbers = make([]OpenNumber, len(o.OpenNumbers))
	for i, t := range o.OpenNumbers {
		b.openNumberIndex[t] = i
		matcherIndex := -1
		if e, ok := t.(*sem.TemplateEnumParam); ok && e.Matcher != nil {
			var err error
			matcherIndex, err = b.matcherIndex(e.Matcher)
			if err != nil {
				return err
			}
		}
		b.openNumbers[i] = OpenNumber{
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
// BuiltinTable.TMatcher or BuiltinTable.NMatcher, respectively.
func (b *overloadBuilder) matcherIndex(n sem.Named) (int, error) {
	switch n := n.(type) {
	case *sem.Type, *sem.TypeMatcher:
		if i, ok := b.TMatcherIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("matcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.TemplateTypeParam:
		if i, ok := b.openTypeIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("openTypeIndex missing entry for %v %T", n.Name, n)
	case *sem.EnumMatcher:
		if i, ok := b.NMatcherIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("matcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.TemplateEnumParam:
		if i, ok := b.openNumberIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("openNumberIndex missing entry for %v %T", n, n)
	case *sem.TemplateNumberParam:
		if i, ok := b.openNumberIndex[n]; ok {
			return i, nil
		}
		return 0, fmt.Errorf("openNumberIndex missing entry for %v %T", n, n)
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
//    A<B<C, D>, E<F, G<H>, I>
// Would return the matcher indices:
//    A, B, C, D, E, F, G, H, I
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

// buildBuiltinTable builds the BuiltinTable from the semantic info
func buildBuiltinTable(s *sem.Sem) (*BuiltinTable, error) {
	b := BuiltinTableBuilder{
		BuiltinTable: BuiltinTable{
			Sem:           s,
			TMatcherIndex: map[sem.Named]int{},
			NMatcherIndex: map[sem.Named]int{},
		},
	}
	b.lut.matcherIndices = lut.New(list.Wrap(&b.MatcherIndices))
	b.lut.openTypes = lut.New(list.Wrap(&b.OpenTypes))
	b.lut.openNumbers = lut.New(list.Wrap(&b.OpenNumbers))
	b.lut.parameters = lut.New(list.Wrap(&b.Parameters))
	b.lut.overloads = lut.New(list.Wrap(&b.Overloads))

	b.layoutMatchers(s)

	for _, f := range s.Functions {
		overloads := make([]Overload, len(f.Overloads))
		overloadDescriptions := make([]string, len(f.Overloads))
		for i, o := range f.Overloads {
			overloadDescriptions[i] = fmt.Sprint(o.Decl)
			var err error
			if overloads[i], err = b.buildOverload(o); err != nil {
				return nil, err
			}
		}

		b.Functions = append(b.Functions, Function{
			OverloadDescriptions: overloadDescriptions,
			NumOverloads:         len(overloads),
			OverloadsOffset:      b.lut.overloads.Add(overloads),
		})
	}

	b.lut.matcherIndices.Compact()
	b.lut.openTypes.Compact()
	b.lut.openNumbers.Compact()
	b.lut.parameters.Compact()
	b.lut.overloads.Compact()

	return &b.BuiltinTable, nil
}
