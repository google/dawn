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

package resolver

import (
	"fmt"
	"sort"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/ast"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/sem"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/tok"
)

type resolver struct {
	a *ast.AST
	s *sem.Sem

	globals           scope
	functions         map[string]*sem.Function
	enumEntryMatchers map[*sem.EnumEntry]*sem.EnumMatcher
}

// Resolve processes the AST
func Resolve(a *ast.AST) (*sem.Sem, error) {
	r := resolver{
		a:                 a,
		s:                 sem.New(),
		globals:           newScope(nil),
		functions:         map[string]*sem.Function{},
		enumEntryMatchers: map[*sem.EnumEntry]*sem.EnumMatcher{},
	}
	// Declare and resolve all the enumerators
	for _, e := range a.Enums {
		if err := r.enum(e); err != nil {
			return nil, err
		}
	}
	// Declare and resolve all the ty types
	for _, p := range a.Types {
		if err := r.ty(p); err != nil {
			return nil, err
		}
	}
	// Declare and resolve the type matchers
	for _, m := range a.Matchers {
		if err := r.matcher(m); err != nil {
			return nil, err
		}
	}
	// Declare and resolve the functions
	for _, f := range a.Functions {
		if err := r.function(f); err != nil {
			return nil, err
		}
	}

	// Calculate the unique parameter names
	r.s.UniqueParameterNames = r.calculateUniqueParameterNames()

	return r.s, nil
}

// enum() resolves an enum declaration.
// The resulting sem.Enum is appended to Sem.Enums, and the enum and all its
// entries are registered with the global scope.
func (r *resolver) enum(e ast.EnumDecl) error {
	s := &sem.Enum{
		Decl: e,
		Name: e.Name,
	}

	// Register the enum
	r.s.Enums = append(r.s.Enums, s)
	if err := r.globals.declare(s, e.Source); err != nil {
		return err
	}

	// Register each of the enum entries
	for _, ast := range e.Entries {
		entry := &sem.EnumEntry{
			Name: ast.Name,
			Enum: s,
		}
		if internal := ast.Decorations.Take("internal"); internal != nil {
			entry.IsInternal = true
			if len(internal.Values) != 0 {
				return fmt.Errorf("%v unexpected value for internal decoration", ast.Source)
			}
		}
		if len(ast.Decorations) != 0 {
			return fmt.Errorf("%v unknown decoration", ast.Decorations[0].Source)
		}
		if err := r.globals.declare(entry, e.Source); err != nil {
			return err
		}
		s.Entries = append(s.Entries, entry)
	}

	return nil
}

// ty() resolves a type declaration.
// The resulting sem.Type is appended to Sem.Types, and the type is registered
// with the global scope.
func (r *resolver) ty(a ast.TypeDecl) error {
	t := &sem.Type{
		Decl: a,
		Name: a.Name,
	}

	// Register the type
	r.s.Types = append(r.s.Types, t)
	if err := r.globals.declare(t, a.Source); err != nil {
		return err
	}

	// Create a new scope for resolving template parameters
	s := newScope(&r.globals)

	// Resolve the type template parameters
	templateParams, err := r.templateParams(&s, a.TemplateParams)
	if err != nil {
		return err
	}
	t.TemplateParams = templateParams

	// Scan for decorations
	if d := a.Decorations.Take("display"); d != nil {
		if len(d.Values) != 1 {
			return fmt.Errorf("%v expected a single value for 'display' decoration", d.Source)
		}
		t.DisplayName = d.Values[0]
	}
	if len(a.Decorations) != 0 {
		return fmt.Errorf("%v unknown decoration", a.Decorations[0].Source)
	}

	return nil
}

// matcher() resolves a match declaration to either a sem.TypeMatcher or
// sem.EnumMatcher.
// The resulting matcher is appended to either Sem.TypeMatchers or
// Sem.EnumMatchers, and is registered with the global scope.
func (r *resolver) matcher(a ast.MatcherDecl) error {
	// Determine whether this is a type matcher or enum matcher by resolving the
	// first option
	firstOption, err := r.lookupNamed(&r.globals, a.Options[0])
	if err != nil {
		return err
	}

	// Resolve to a sem.TypeMatcher or a sem.EnumMatcher
	switch firstOption := firstOption.(type) {
	case *sem.Type:
		options := map[sem.Named]tok.Source{}
		m := &sem.TypeMatcher{
			Decl: a,
			Name: a.Name,
		}

		// Register the matcher
		r.s.TypeMatchers = append(r.s.TypeMatchers, m)
		if err := r.globals.declare(m, a.Source); err != nil {
			return err
		}

		// Resolve each of the types in the options list
		for _, ast := range m.Decl.Options {
			ty, err := r.lookupType(&r.globals, ast)
			if err != nil {
				return err
			}
			m.Types = append(m.Types, ty)
			if s, dup := options[ty]; dup {
				return fmt.Errorf("%v duplicate option '%v' in matcher\nFirst declared here: %v", ast.Source, ast.Name, s)
			}
			options[ty] = ast.Source
		}

		return nil

	case *sem.EnumEntry:
		enum := firstOption.Enum
		m := &sem.EnumMatcher{
			Decl: a,
			Name: a.Name,
			Enum: enum,
		}

		// Register the matcher
		r.s.EnumMatchers = append(r.s.EnumMatchers, m)
		if err := r.globals.declare(m, a.Source); err != nil {
			return err
		}

		// Resolve each of the enums in the options list
		for _, ast := range m.Decl.Options {
			entry := enum.FindEntry(ast.Name)
			if entry == nil {
				return fmt.Errorf("%v enum '%v' does not contain '%v'", ast.Source, enum.Name, ast.Name)
			}
			m.Options = append(m.Options, entry)
		}

		return nil
	}
	return fmt.Errorf("'%v' cannot be used for matcher", a.Name)
}

// function() resolves a function overload declaration.
// The the first overload for the function creates and appends the sem.Function
// to Sem.Functions. Subsequent overloads append their resolved overload to the
// sem.Function.Overloads list.
func (r *resolver) function(a ast.FunctionDecl) error {
	// If this is the first overload of the function, create and register the
	// semantic function.
	f := r.functions[a.Name]
	if f == nil {
		f = &sem.Function{Name: a.Name}
		r.functions[a.Name] = f
		r.s.Functions = append(r.s.Functions, f)
	}

	// Create a new scope for resolving template parameters
	s := newScope(&r.globals)

	// Resolve the declared template parameters
	templateParams, err := r.templateParams(&s, a.TemplateParams)
	if err != nil {
		return err
	}

	// Construct the semantic overload
	overload := &sem.Overload{
		Decl:           a,
		Function:       f,
		Parameters:     make([]sem.Parameter, len(a.Parameters)),
		TemplateParams: templateParams,
	}

	// Process overload decorations
	if stageDeco := a.Decorations.Take("stage"); stageDeco != nil {
		for stageDeco != nil {
			for _, stage := range stageDeco.Values {
				switch stage {
				case "vertex":
					overload.CanBeUsedInStage.Vertex = true
				case "fragment":
					overload.CanBeUsedInStage.Fragment = true
				case "compute":
					overload.CanBeUsedInStage.Compute = true
				default:
					return fmt.Errorf("%v unknown stage '%v'", stageDeco.Source, stage)
				}
			}
			stageDeco = a.Decorations.Take("stage")
		}
	} else {
		overload.CanBeUsedInStage = sem.StageUses{
			Vertex:   true,
			Fragment: true,
			Compute:  true,
		}
	}
	if deprecated := a.Decorations.Take("deprecated"); deprecated != nil {
		overload.IsDeprecated = true
		if len(deprecated.Values) != 0 {
			return fmt.Errorf("%v unexpected value for deprecated decoration", deprecated.Source)
		}
	}
	if len(a.Decorations) != 0 {
		return fmt.Errorf("%v unknown decoration", a.Decorations[0].Source)
	}

	// Append the overload to the function
	f.Overloads = append(f.Overloads, overload)

	// Sort the template parameters by resolved type. Append these to
	// sem.Overload.OpenTypes or sem.Overload.OpenNumbers based on their kind.
	for _, param := range templateParams {
		switch param := param.(type) {
		case *sem.TemplateTypeParam:
			overload.OpenTypes = append(overload.OpenTypes, param)
		case *sem.TemplateEnumParam, *sem.TemplateNumberParam:
			overload.OpenNumbers = append(overload.OpenNumbers, param)
		}
	}

	// Update high-water marks of open types / numbers
	if r.s.MaxOpenTypes < len(overload.OpenTypes) {
		r.s.MaxOpenTypes = len(overload.OpenTypes)
	}
	if r.s.MaxOpenNumbers < len(overload.OpenNumbers) {
		r.s.MaxOpenNumbers = len(overload.OpenNumbers)
	}

	// Resolve the parameters
	for i, p := range a.Parameters {
		usage, err := r.fullyQualifiedName(&s, p.Type)
		if err != nil {
			return err
		}
		overload.Parameters[i] = sem.Parameter{
			Name: p.Name,
			Type: usage,
		}
	}

	// Resolve the return type
	if a.ReturnType != nil {
		usage, err := r.fullyQualifiedName(&s, *a.ReturnType)
		if err != nil {
			return err
		}
		switch usage.Target.(type) {
		case *sem.Type, *sem.TemplateTypeParam:
			overload.ReturnType = &usage
		default:
			return fmt.Errorf("%v cannot use '%v' as return type. Must be a type or template type", a.ReturnType.Source, a.ReturnType.Name)
		}
	}

	return nil
}

// fullyQualifiedName() resolves the ast.TemplatedName to a sem.FullyQualifiedName.
func (r *resolver) fullyQualifiedName(s *scope, arg ast.TemplatedName) (sem.FullyQualifiedName, error) {
	target, err := r.lookupNamed(s, arg)
	if err != nil {
		return sem.FullyQualifiedName{}, err
	}

	if entry, ok := target.(*sem.EnumEntry); ok {
		// The target resolved to an enum entry.
		// Automagically transform this into a synthetic matcher with a single
		// option. i.e.
		// This:
		//   enum E{ a b c }
		//   fn F(b)
		// Becomes:
		//   enum E{ a b c }
		//   matcher b
		//   fn F(b)
		// We don't really care right now that we have a symbol collision
		// between E.b and b, as the generators return different names for
		// these.
		matcher, ok := r.enumEntryMatchers[entry]
		if !ok {
			matcher = &sem.EnumMatcher{
				Name:    entry.Name,
				Enum:    entry.Enum,
				Options: []*sem.EnumEntry{entry},
			}
			r.enumEntryMatchers[entry] = matcher
			r.s.EnumMatchers = append(r.s.EnumMatchers, matcher)
		}
		target = matcher
	}

	fqn := sem.FullyQualifiedName{
		Target:            target,
		TemplateArguments: make([]interface{}, len(arg.TemplateArgs)),
	}
	for i, a := range arg.TemplateArgs {
		arg, err := r.fullyQualifiedName(s, a)
		if err != nil {
			return sem.FullyQualifiedName{}, err
		}
		fqn.TemplateArguments[i] = arg
	}
	return fqn, nil
}

// templateParams() resolves the ast.TemplateParams into list of sem.TemplateParam.
// Each sem.TemplateParam is registered with the scope s.
func (r *resolver) templateParams(s *scope, l ast.TemplateParams) ([]sem.TemplateParam, error) {
	out := []sem.TemplateParam{}
	for _, ast := range l {
		param, err := r.templateParam(ast)
		if err != nil {
			return nil, err
		}
		s.declare(param, ast.Source)
		out = append(out, param)
	}
	return out, nil
}

// templateParams() resolves the ast.TemplateParam into sem.TemplateParam, which
// is either a sem.TemplateEnumParam or a sem.TemplateTypeParam.
func (r *resolver) templateParam(a ast.TemplateParam) (sem.TemplateParam, error) {
	if a.Type.Name == "num" {
		return &sem.TemplateNumberParam{Name: a.Name}, nil
	}

	if a.Type.Name != "" {
		resolved, err := r.lookupNamed(&r.globals, a.Type)
		if err != nil {
			return nil, err
		}
		switch r := resolved.(type) {
		case *sem.Enum:
			return &sem.TemplateEnumParam{Name: a.Name, Enum: r}, nil
		case *sem.EnumMatcher:
			return &sem.TemplateEnumParam{Name: a.Name, Enum: r.Enum, Matcher: r}, nil
		case *sem.TypeMatcher:
			return &sem.TemplateTypeParam{Name: a.Name, Type: r}, nil
		default:
			return nil, fmt.Errorf("%v invalid template parameter type '%v'", a.Source, a.Type.Name)
		}
	}

	return &sem.TemplateTypeParam{Name: a.Name}, nil
}

// lookupType() searches the scope `s` and its ancestors for the sem.Type with
// the given name.
func (r *resolver) lookupType(s *scope, a ast.TemplatedName) (*sem.Type, error) {
	resolved, err := r.lookupNamed(s, a)
	if err != nil {
		return nil, err
	}
	// Something with the given name was found...
	if ty, ok := resolved.(*sem.Type); ok {
		return ty, nil
	}
	// ... but that something was not a sem.Type
	return nil, fmt.Errorf("%v '%v' resolves to %v but type is expected", a.Source, a.Name, describe(resolved))
}

// lookupNamed() searches `s` and its ancestors for the sem.Named object with
// the given name. If there are template arguments for the name `a`, then
// lookupNamed() performs basic validation that those arguments can be passed
// to the named object.
func (r *resolver) lookupNamed(s *scope, a ast.TemplatedName) (sem.Named, error) {
	target := s.lookup(a.Name)
	if target == nil {
		return nil, fmt.Errorf("%v cannot resolve '%v'", a.Source, a.Name)
	}

	// Something with the given name was found...
	var params []sem.TemplateParam
	var ty sem.ResolvableType
	switch target := target.object.(type) {
	case *sem.Type:
		ty = target
		params = target.TemplateParams
	case *sem.TypeMatcher:
		ty = target
		params = target.TemplateParams
	case sem.TemplateParam:
		if len(a.TemplateArgs) != 0 {
			return nil, fmt.Errorf("%v '%v' template parameters do not accept template arguments", a.Source, a.Name)
		}
		return target.(sem.Named), nil
	case sem.Named:
		return target, nil
	default:
		panic(fmt.Errorf("Unknown resolved type %T", target))
	}
	// ... and that something takes template parameters
	// Check the number of templated name template arguments match the number of
	// templated parameters for the target.
	args := a.TemplateArgs
	if len(params) != len(args) {
		return nil, fmt.Errorf("%v '%v' requires %d template arguments, but %d were provided", a.Source, a.Name, len(params), len(args))
	}

	// Check templated name template argument kinds match the parameter kinds
	for i, ast := range args {
		param := params[i]
		arg, err := r.lookupNamed(s, args[i])
		if err != nil {
			return nil, err
		}

		if err := checkCompatible(arg, param); err != nil {
			return nil, fmt.Errorf("%v %w", ast.Source, err)
		}
	}
	return ty, nil
}

// calculateUniqueParameterNames() iterates over all the parameters of all
// overloads, calculating the list of unique parameter names
func (r *resolver) calculateUniqueParameterNames() []string {
	set := map[string]struct{}{"": {}}
	names := []string{}
	for _, f := range r.s.Functions {
		for _, o := range f.Overloads {
			for _, p := range o.Parameters {
				if _, dup := set[p.Name]; !dup {
					set[p.Name] = struct{}{}
					names = append(names, p.Name)
				}
			}
		}
	}
	sort.Strings(names)
	return names
}

// describe() returns a string describing a sem.Named
func describe(n sem.Named) string {
	switch n := n.(type) {
	case *sem.Type:
		return "type '" + n.Name + "'"
	case *sem.TypeMatcher:
		return "type matcher '" + n.Name + "'"
	case *sem.Enum:
		return "enum '" + n.Name + "'"
	case *sem.EnumMatcher:
		return "enum matcher '" + n.Name + "'"
	case *sem.TemplateTypeParam:
		return "template type"
	case *sem.TemplateEnumParam:
		return "template enum '" + n.Enum.Name + "'"
	case *sem.EnumEntry:
		return "enum entry '" + n.Enum.Name + "." + n.Name + "'"
	case *sem.TemplateNumberParam:
		return "template number"
	default:
		panic(fmt.Errorf("unhandled type %T", n))
	}
}

// checkCompatible() returns an error if `arg` cannot be used as an argument for
// a parameter of `param`.
func checkCompatible(arg, param sem.Named) error {
	// asEnum() returns the underlying sem.Enum if n is a enum matcher,
	// templated enum parameter or an enum entry, otherwise nil
	asEnum := func(n sem.Named) *sem.Enum {
		switch n := n.(type) {
		case *sem.EnumMatcher:
			return n.Enum
		case *sem.TemplateEnumParam:
			return n.Enum
		case *sem.EnumEntry:
			return n.Enum
		default:
			return nil
		}
	}

	if arg := asEnum(arg); arg != nil {
		param := asEnum(param)
		if arg == param {
			return nil
		}
	}

	anyNumber := "any number"
	// asNumber() returns anyNumber if n is a TemplateNumberParam.
	// TODO(bclayton): Once we support number ranges [e.g.: fn F<N: 1..4>()], we
	// should check number ranges are compatible
	asNumber := func(n sem.Named) interface{} {
		switch n.(type) {
		case *sem.TemplateNumberParam:
			return anyNumber
		default:
			return nil
		}
	}

	if arg := asNumber(arg); arg != nil {
		param := asNumber(param)
		if arg == param {
			return nil
		}
	}

	anyType := &sem.Type{}
	// asNumber() returns the sem.Type, sem.TypeMatcher if the named object
	// resolves to one of these, or anyType if n is a unconstrained template
	// type parameter.
	asResolvableType := func(n sem.Named) sem.ResolvableType {
		switch n := n.(type) {
		case *sem.TemplateTypeParam:
			if n.Type != nil {
				return n.Type
			}
			return anyType
		case *sem.Type:
			return n
		case *sem.TypeMatcher:
			return n
		default:
			return nil
		}
	}

	if arg := asResolvableType(arg); arg != nil {
		param := asResolvableType(param)
		if arg == param || param == anyType {
			return nil
		}
	}

	return fmt.Errorf("cannot use %v as %v", describe(arg), describe(param))
}

// scope is a basic hierarchical name to object table
type scope struct {
	objects map[string]objectAndSource
	parent  *scope
}

// objectAndSource is a sem.Named object with a source
type objectAndSource struct {
	object sem.Named
	source tok.Source
}

// newScope returns a newly initalized scope
func newScope(parent *scope) scope {
	return scope{objects: map[string]objectAndSource{}, parent: parent}
}

// lookup() searches the scope and then its parents for the symbol with the
// given name.
func (s *scope) lookup(name string) *objectAndSource {
	if o, found := s.objects[name]; found {
		return &o
	}
	if s.parent == nil {
		return nil
	}
	return s.parent.lookup(name)
}

// declare() declares the symbol with the given name, erroring on symbol
// collision.
func (s *scope) declare(object sem.Named, source tok.Source) error {
	name := object.GetName()
	if existing := s.lookup(name); existing != nil {
		return fmt.Errorf("%v '%v' already declared\nFirst declared here: %v", source, name, existing.source)
	}
	s.objects[name] = objectAndSource{object, source}
	return nil
}
