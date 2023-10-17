// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Package gen holds types and helpers for generating templated code from the
// intrinsic.def file.
//
// Used by tools/src/cmd/gen/main.go
package gen

import (
	"fmt"
	"strings"

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

	TypeMatcherIndices        []int            // kTypeMatcherIndices table content
	NumberMatcherIndices      []int            // kNumberMatcherIndices table content
	TemplateTypes             []TemplateType   // kTemplateTypes table content
	TemplateNumbers           []TemplateNumber // kTemplateNumbers table content
	Parameters                []Parameter      // kParameters table content
	Overloads                 []Overload       // kOverloads table content
	Builtins                  []Intrinsic      // kBuiltins table content
	UnaryOperators            []Intrinsic      // kUnaryOperators table content
	BinaryOperators           []Intrinsic      // kBinaryOperators table content
	ConstructorsAndConverters []Intrinsic      // kInitializersAndConverters table content
	ConstEvalFunctions        []string         // kConstEvalFunctions table content
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

	// Index into IntrinsicTable.TypeMatcherIndices, beginning the list of matchers
	// required to match the parameter type.
	// The matcher indices index into IntrinsicTable::TMatchers.
	// These indices are consumed by the matchers themselves.
	TypeMatcherIndicesOffset int

	// Index into IntrinsicTable.NumberMatcherIndices.
	// The matcher indices index into IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	NumberMatcherIndicesOffset int
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
	TemplateTypesOffset int
	// Index to the first template number in IntrinsicTable.TemplateNumbers
	TemplateNumbersOffset int
	// Index to the first parameter in IntrinsicTable.Parameters
	ParametersOffset int
	// Index into IntrinsicTable.TypeMatcherIndices, beginning the list of matchers
	// required to match the return type.
	// The matcher indices index into IntrinsicTable::TMatchers.
	// These indices are consumed by the matchers themselves.
	ReturnTypeMatcherIndicesOffset int
	// Index into IntrinsicTable.NumberMatcherIndices.
	// The matcher indices index into IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	ReturnNumberMatcherIndicesOffset int
	// Index into IntrinsicTable.ConstEvalFunctions.
	ConstEvalFunctionOffset int
	// StageUses describes the stages an overload can be used in
	CanBeUsedInStage sem.StageUses
	// True if the overload is marked as @must_use
	MustUse bool
	// True if the overload is marked as deprecated
	IsDeprecated bool
	// The kind of overload
	Kind string
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
		typeMatcherIndices       lut.LUT[int]
		numberMatcherIndices     lut.LUT[int]
		templateTypes            lut.LUT[TemplateType]
		templateNumbers          lut.LUT[TemplateNumber]
		constEvalFunctionIndices lut.LUT[string]
		parameters               lut.LUT[Parameter]
		overloads                lut.LUT[Overload]
	}
}

type parameterBuilder struct {
	usage                      string
	typeMatcherIndicesOffset   *int
	numberMatcherIndicesOffset *int
}

// Helper for building a single overload
type overloadBuilder struct {
	*IntrinsicTableBuilder
	// The overload being built
	overload *sem.Overload
	// Maps TemplateParam to index in templateTypes
	templateTypeIndex map[sem.TemplateParam]int
	// Maps TemplateParam to index in templateNumbers
	templateNumberIndex map[sem.TemplateParam]int
	// Template types used by the overload
	templateTypes []TemplateType
	// Index to the first template type in IntrinsicTable.TemplateTypes
	templateTypesOffset *int
	// Template numbers used by the overload
	templateNumbers []TemplateNumber
	// Index to the first template number in IntrinsicTable.TemplateNumbers
	templateNumbersOffset *int
	// Builders for all parameters
	parameterBuilders []parameterBuilder
	// Index to the first parameter in IntrinsicTable.Parameters
	parametersOffset *int
	// Index into IntrinsicTable.ConstEvalFunctions
	constEvalFunctionOffset *int
	// Index into IntrinsicTable.TypeMatcherIndices, beginning the list of
	// matchers required to match the return type.
	// The matcher indices index into IntrinsicTable::TMatchers.
	// These indices are consumed by the matchers themselves.
	returnTypeMatcherIndicesOffset *int
	// Index into IntrinsicTable.NumberMatcherIndices.
	// The matcher indices index into IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	returnNumberMatcherIndicesOffset *int
}

// layoutMatchers assigns each of the TMatchers and NMatchers a unique index.
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

func (b *IntrinsicTableBuilder) newOverloadBuilder(o *sem.Overload) *overloadBuilder {
	return &overloadBuilder{
		IntrinsicTableBuilder: b,
		overload:              o,
		templateTypeIndex:     map[sem.TemplateParam]int{},
		templateNumberIndex:   map[sem.TemplateParam]int{},
	}
}

// processStage0 begins processing of the overload.
// Preconditions:
// - Must be called before any LUTs are compacted.
// Populates:
// - b.templateTypes
// - b.templateTypesOffset
// - b.templateNumbers
// - b.templateNumbersOffset
// - b.parameterBuilders
// - b.returnTypeMatcherIndicesOffset
// - b.returnNumberMatcherIndicesOffset
// - b.constEvalFunctionOffset
func (b *overloadBuilder) processStage0() error {
	b.templateTypes = make([]TemplateType, len(b.overload.TemplateTypes))
	for i, t := range b.overload.TemplateTypes {
		b.templateTypeIndex[t] = i
		matcherIndex := -1
		if t.Type != nil {
			tys, nums, err := b.matcherIndices(t.Type)
			if err != nil {
				return err
			}
			if len(tys) != 1 || len(nums) != 0 {
				panic("unexpected result of matcherIndices()")
			}
			matcherIndex = tys[0]
		}
		b.templateTypes[i] = TemplateType{
			Name:         t.Name,
			MatcherIndex: matcherIndex,
		}
	}
	b.templateTypesOffset = b.lut.templateTypes.Add(b.templateTypes)

	b.templateNumbers = make([]TemplateNumber, len(b.overload.TemplateNumbers))
	for i, t := range b.overload.TemplateNumbers {
		b.templateNumberIndex[t] = i
		matcherIndex := -1
		if e, ok := t.(*sem.TemplateEnumParam); ok && e.Matcher != nil {
			tys, nums, err := b.matcherIndices(e.Matcher)
			if err != nil {
				return err
			}
			if len(tys) != 0 || len(nums) != 1 {
				panic("unexpected result of matcherIndices()")
			}
			matcherIndex = nums[0]
		}
		b.templateNumbers[i] = TemplateNumber{
			Name:         t.GetName(),
			MatcherIndex: matcherIndex,
		}
	}
	b.templateNumbersOffset = b.lut.templateNumbers.Add(b.templateNumbers)

	if b.overload.ReturnType != nil {
		typeIndices, numberIndices, err := b.collectMatcherIndices(*b.overload.ReturnType)
		if err != nil {
			return err
		}
		b.returnTypeMatcherIndicesOffset = b.lut.typeMatcherIndices.Add(typeIndices)
		b.returnNumberMatcherIndicesOffset = b.lut.numberMatcherIndices.Add(numberIndices)
	}

	b.parameterBuilders = make([]parameterBuilder, len(b.overload.Parameters))
	for i, p := range b.overload.Parameters {
		typeIndices, numberIndices, err := b.collectMatcherIndices(p.Type)
		if err != nil {
			return err
		}

		b.parameterBuilders[i] = parameterBuilder{
			usage:                      p.Name,
			typeMatcherIndicesOffset:   b.lut.typeMatcherIndices.Add(typeIndices),
			numberMatcherIndicesOffset: b.lut.numberMatcherIndices.Add(numberIndices),
		}
	}

	if b.overload.ConstEvalFunction != "" {
		b.constEvalFunctionOffset = b.lut.constEvalFunctionIndices.Add([]string{b.overload.ConstEvalFunction})
	}

	return nil
}

// processStage1 builds the Parameters  used by the overload
// Must only be called after the following LUTs have been compacted:
// - b.lut.typeMatcherIndices
// - b.lut.numberMatcherIndices
// - b.lut.templateTypes
// - b.lut.templateNumbers
// Populates:
// - b.parametersOffset
func (b *overloadBuilder) processStage1() error {
	parameters := make([]Parameter, len(b.parameterBuilders))
	for i, pb := range b.parameterBuilders {
		parameters[i] = Parameter{
			Usage:                      pb.usage,
			TypeMatcherIndicesOffset:   loadOrMinusOne(pb.typeMatcherIndicesOffset),
			NumberMatcherIndicesOffset: loadOrMinusOne(pb.numberMatcherIndicesOffset),
		}
	}
	b.parametersOffset = b.lut.parameters.Add(parameters)
	return nil
}

func (b *overloadBuilder) build() (Overload, error) {
	return Overload{
		NumParameters:                    len(b.parameterBuilders),
		NumTemplateTypes:                 len(b.templateTypes),
		NumTemplateNumbers:               len(b.templateNumbers),
		TemplateTypesOffset:              loadOrMinusOne(b.templateTypesOffset),
		TemplateNumbersOffset:            loadOrMinusOne(b.templateNumbersOffset),
		ParametersOffset:                 loadOrMinusOne(b.parametersOffset),
		ConstEvalFunctionOffset:          loadOrMinusOne(b.constEvalFunctionOffset),
		ReturnTypeMatcherIndicesOffset:   loadOrMinusOne(b.returnTypeMatcherIndicesOffset),
		ReturnNumberMatcherIndicesOffset: loadOrMinusOne(b.returnNumberMatcherIndicesOffset),
		CanBeUsedInStage:                 b.overload.CanBeUsedInStage,
		MustUse:                          b.overload.MustUse,
		IsDeprecated:                     b.overload.IsDeprecated,
		Kind:                             string(b.overload.Decl.Kind),
	}, nil
}

// matcherIndex returns the matcher indices into IntrinsicTable.TMatcher and
// IntrinsicTable.NMatcher, respectively for the given named entity.
func (b *overloadBuilder) matcherIndices(n sem.Named) (types, numbers []int, err error) {
	switch n := n.(type) {
	case *sem.Type, *sem.TypeMatcher:
		if i, ok := b.TMatcherIndex[n]; ok {
			return []int{i}, nil, nil
		}
		return nil, nil, fmt.Errorf("matcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.TemplateTypeParam:
		if i, ok := b.templateTypeIndex[n]; ok {
			return []int{i}, nil, nil
		}
		return nil, nil, fmt.Errorf("templateTypeIndex missing entry for %v %T", n.Name, n)
	case *sem.EnumMatcher:
		if i, ok := b.NMatcherIndex[n]; ok {
			return nil, []int{i}, nil
		}
		return nil, nil, fmt.Errorf("matcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.TemplateEnumParam:
		if i, ok := b.templateNumberIndex[n]; ok {
			return nil, []int{i}, nil
		}
		return nil, nil, fmt.Errorf("templateNumberIndex missing entry for %v %T", n, n)
	case *sem.TemplateNumberParam:
		if i, ok := b.templateNumberIndex[n]; ok {
			return nil, []int{i}, nil
		}
		return nil, nil, fmt.Errorf("templateNumberIndex missing entry for %v %T", n, n)
	default:
		return nil, nil, fmt.Errorf("overload.matcherIndices() does not handle %v %T", n, n)
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
func (b *overloadBuilder) collectMatcherIndices(fqn sem.FullyQualifiedName) (tys, nums []int, err error) {
	tys, nums, err = b.matcherIndices(fqn.Target)
	if err != nil {
		return nil, nil, err
	}
	for _, arg := range fqn.TemplateArguments {
		typeIndices, numberIndices, err := b.collectMatcherIndices(arg.(sem.FullyQualifiedName))
		if err != nil {
			return nil, nil, err
		}
		tys = append(tys, typeIndices...)
		nums = append(nums, numberIndices...)
	}
	return tys, nums, nil
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
	b.layoutMatchers(s)

	intrinsicGroups := []struct {
		in  []*sem.Intrinsic
		out *[]Intrinsic
	}{
		{s.Builtins, &b.Builtins},
		{s.UnaryOperators, &b.UnaryOperators},
		{s.BinaryOperators, &b.BinaryOperators},
		{s.ConstructorsAndConverters, &b.ConstructorsAndConverters},
	}

	// Create an overload builder for every overload
	overloadToBuilder := map[*sem.Overload]*overloadBuilder{}
	overloadBuilders := []*overloadBuilder{}
	for _, intrinsics := range intrinsicGroups {
		for _, f := range intrinsics.in {
			for _, o := range f.Overloads {
				builder := b.newOverloadBuilder(o)
				overloadToBuilder[o] = builder
				overloadBuilders = append(overloadBuilders, builder)
			}
		}
	}

	// Perform the 'stage-0' processing of the overloads
	b.lut.typeMatcherIndices = lut.New[int]()
	b.lut.numberMatcherIndices = lut.New[int]()
	b.lut.templateTypes = lut.New[TemplateType]()
	b.lut.templateNumbers = lut.New[TemplateNumber]()
	b.lut.constEvalFunctionIndices = lut.New[string]()
	for _, b := range overloadBuilders {
		b.processStage0()
	}

	// Compact type and number LUTs
	b.TypeMatcherIndices = b.lut.typeMatcherIndices.Compact()
	b.NumberMatcherIndices = b.lut.numberMatcherIndices.Compact()
	b.TemplateTypes = b.lut.templateTypes.Compact()
	b.TemplateNumbers = b.lut.templateNumbers.Compact()
	b.ConstEvalFunctions = b.lut.constEvalFunctionIndices.Compact()
	// Clear the compacted LUTs to prevent use-after-compaction
	b.lut.typeMatcherIndices = nil
	b.lut.numberMatcherIndices = nil
	b.lut.templateTypes = nil
	b.lut.templateNumbers = nil
	b.lut.constEvalFunctionIndices = nil

	// Perform the 'stage-1' processing of the overloads
	b.lut.parameters = lut.New[Parameter]()
	for _, b := range overloadBuilders {
		b.processStage1()
	}
	b.Parameters = b.lut.parameters.Compact()
	b.lut.parameters = nil

	// Build the Intrinsics
	b.lut.overloads = lut.New[Overload]()
	for _, intrinsics := range intrinsicGroups {
		out := make([]Intrinsic, len(intrinsics.in))
		for i, f := range intrinsics.in {
			overloads := make([]Overload, len(f.Overloads))
			overloadDescriptions := make([]string, len(f.Overloads))
			for i, o := range f.Overloads {
				overloadDescriptions[i] = fmt.Sprint(o.Decl)
				overload, err := overloadToBuilder[o].build()
				if err != nil {
					return nil, err
				}
				overloads[i] = overload
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

	b.Overloads = b.lut.overloads.Compact()

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
	case "mat2x2", "mat2x3", "mat2x4",
		"mat3x2", "mat3x3", "mat3x4",
		"mat4x2", "mat4x3", "mat4x4":
		return DeepestElementType(fqn.TemplateArguments[0].(sem.FullyQualifiedName))
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

// IsHostShareable returns true if the FullyQualifiedName refers to a type that is host-sharable.
// See https://www.w3.org/TR/WGSL/#host-shareable-types
func IsHostShareable(fqn sem.FullyQualifiedName) bool {
	return IsDeclarable(fqn) && DeepestElementType(fqn).Target.GetName() != "bool"
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

// OverloadUsesReadWriteStorageTexture returns true if the overload uses a read-only or read-write
// storage texture.
func OverloadUsesReadWriteStorageTexture(overload sem.Overload) bool {
	for _, param := range overload.Parameters {
		if strings.HasPrefix(param.Type.Target.GetName(), "texture_storage") {
			access := param.Type.TemplateArguments[1].(sem.FullyQualifiedName).Target.GetName()
			if access == "read" || access == "read_write" {
				return true
			}
		}
	}
	return false
}

func loadOrMinusOne(p *int) int {
	if p != nil {
		return *p
	}
	return -1
}
