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
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

// Permuter generates permutations of intrinsic overloads
type Permuter struct {
	sem      *sem.Sem
	allTypes []sem.FullyQualifiedName
}

// NewPermuter returns a new initialized Permuter
func NewPermuter(s *sem.Sem) (*Permuter, error) {
	// allTypes are the list of FQNs that are used for unconstrained types
	allTypes := []sem.FullyQualifiedName{}
	for _, ty := range s.Types {
		if len(ty.TemplateParams) > 0 {
			// Ignore aggregate types for now.
			// TODO(bclayton): Support a limited set of aggregate types
			continue
		}
		allTypes = append(allTypes, sem.FullyQualifiedName{Target: ty})
	}
	return &Permuter{
		sem:      s,
		allTypes: allTypes,
	}, nil
}

// Permutation describes a single permutation of an overload
type Permutation struct {
	sem.Overload        // The permutated overload signature
	Desc         string // Description of the overload
	Hash         string // Hash of the overload
}

// Permute generates a set of permutations for the given intrinsic overload
func (p *Permuter) Permute(overload *sem.Overload) ([]Permutation, error) {
	state := permutationState{
		Permuter:        p,
		templateTypes:   map[sem.TemplateParam]sem.FullyQualifiedName{},
		templateNumbers: map[sem.TemplateParam]interface{}{},
		parameters:      map[int]sem.FullyQualifiedName{},
	}

	out := []Permutation{}

	// Map of hash to permutation description. Used to detect collisions.
	hashes := map[string]string{}

	// permutate appends a permutation to out.
	// permutate may be chained to generate N-dimensional permutations.
	permutate := func() error {
		o := sem.Overload{
			Decl:             overload.Decl,
			Intrinsic:        overload.Intrinsic,
			CanBeUsedInStage: overload.CanBeUsedInStage,
		}
		for i, p := range overload.Parameters {
			ty := state.parameters[i]
			if !validate(ty, &o.CanBeUsedInStage) {
				return nil
			}
			o.Parameters = append(o.Parameters, sem.Parameter{
				Name:      p.Name,
				Type:      ty,
				IsConst:   p.IsConst,
				TestValue: p.TestValue,
			})
		}
		if overload.ReturnType != nil {
			retTys, err := state.permutateFQN(*overload.ReturnType)
			if err != nil {
				return fmt.Errorf("while permutating return type: %w", err)
			}
			if len(retTys) != 1 {
				return fmt.Errorf("result type not pinned")
			}
			o.ReturnType = &retTys[0]
		}
		desc := fmt.Sprint(o)
		hash := sha256.Sum256([]byte(desc))
		const hashLength = 6
		shortHash := hex.EncodeToString(hash[:])[:hashLength]
		out = append(out, Permutation{
			Overload: o,
			Desc:     desc,
			Hash:     shortHash,
		})

		// Check for hash collisions
		if existing, collision := hashes[shortHash]; collision {
			return fmt.Errorf("hash '%v' collision between %v and %v\nIncrease hashLength in %v",
				shortHash, existing, desc, fileutils.ThisLine())
		}
		hashes[shortHash] = desc
		return nil
	}
	for i, param := range overload.Parameters {
		i, param := i, param // Capture iterator values for anonymous function
		next := permutate    // Permutation chaining
		permutate = func() error {
			permutations, err := state.permutateFQN(param.Type)
			if err != nil {
				return fmt.Errorf("while processing parameter %v: %w", i, err)
			}
			if len(permutations) == 0 {
				return fmt.Errorf("parameter %v has no permutations", i)
			}
			for _, fqn := range permutations {
				state.parameters[i] = fqn
				if err := next(); err != nil {
					return err
				}
			}
			return nil
		}
	}
	for _, t := range overload.TemplateParams {
		next := permutate // Permutation chaining
		switch t := t.(type) {
		case *sem.TemplateTypeParam:
			types := p.allTypes
			if t.Type != nil {
				var err error
				types, err = state.permutateFQN(sem.FullyQualifiedName{Target: t.Type})
				if err != nil {
					return nil, fmt.Errorf("while permutating template types: %w", err)
				}
			}
			if len(types) == 0 {
				return nil, fmt.Errorf("template type %v has no permutations", t.Name)
			}
			permutate = func() error {
				for _, ty := range types {
					state.templateTypes[t] = ty
					if err := next(); err != nil {
						return err
					}
				}
				return nil
			}
		case *sem.TemplateEnumParam:
			var permutations []sem.FullyQualifiedName
			var err error
			if t.Matcher != nil {
				permutations, err = state.permutateFQN(sem.FullyQualifiedName{Target: t.Matcher})
			} else {
				permutations, err = state.permutateFQN(sem.FullyQualifiedName{Target: t.Enum})
			}
			if err != nil {
				return nil, fmt.Errorf("while permutating template numbers: %w", err)
			}
			if len(permutations) == 0 {
				return nil, fmt.Errorf("template type %v has no permutations", t.Name)
			}
			permutate = func() error {
				for _, n := range permutations {
					state.templateNumbers[t] = n
					if err := next(); err != nil {
						return err
					}
				}
				return nil
			}
		case *sem.TemplateNumberParam:
			// Currently all open numbers are used for vector / matrices
			permutations := []int{2, 3, 4}
			permutate = func() error {
				for _, n := range permutations {
					state.templateNumbers[t] = n
					if err := next(); err != nil {
						return err
					}
				}
				return nil
			}
		}
	}

	if err := permutate(); err != nil {
		return nil, fmt.Errorf("%v %v %w\nState: %v", overload.Decl.Source, overload.Decl, err, state)
	}

	return out, nil
}

type permutationState struct {
	*Permuter
	templateTypes   map[sem.TemplateParam]sem.FullyQualifiedName
	templateNumbers map[sem.TemplateParam]interface{}
	parameters      map[int]sem.FullyQualifiedName
}

func (s permutationState) String() string {
	sb := &strings.Builder{}
	sb.WriteString("Template types:\n")
	for ct, ty := range s.templateTypes {
		fmt.Fprintf(sb, "  %v: %v\n", ct.GetName(), ty)
	}
	sb.WriteString("Template numbers:\n")
	for cn, v := range s.templateNumbers {
		fmt.Fprintf(sb, "  %v: %v\n", cn.GetName(), v)
	}
	return sb.String()
}

func (s *permutationState) permutateFQN(in sem.FullyQualifiedName) ([]sem.FullyQualifiedName, error) {
	args := append([]interface{}{}, in.TemplateArguments...)
	out := []sem.FullyQualifiedName{}

	// permutate appends a permutation to out.
	// permutate may be chained to generate N-dimensional permutations.
	var permutate func() error

	switch target := in.Target.(type) {
	case *sem.Type:
		permutate = func() error {
			out = append(out, sem.FullyQualifiedName{Target: in.Target, TemplateArguments: args})
			args = append([]interface{}{}, in.TemplateArguments...)
			return nil
		}
	case sem.TemplateParam:
		if ty, ok := s.templateTypes[target]; ok {
			permutate = func() error {
				out = append(out, ty)
				return nil
			}
		} else {
			return nil, fmt.Errorf("'%v' was not found in templateTypes", target.GetName())
		}
	case *sem.TypeMatcher:
		permutate = func() error {
			for _, ty := range target.Types {
				out = append(out, sem.FullyQualifiedName{Target: ty})
			}
			return nil
		}
	case *sem.EnumMatcher:
		permutate = func() error {
			for _, o := range target.Options {
				if !o.IsInternal {
					out = append(out, sem.FullyQualifiedName{Target: o})
				}
			}
			return nil
		}
	case *sem.Enum:
		permutate = func() error {
			for _, e := range target.Entries {
				if !e.IsInternal {
					out = append(out, sem.FullyQualifiedName{Target: e})
				}
			}
			return nil
		}
	default:
		return nil, fmt.Errorf("unhandled target type: %T", in.Target)
	}

	for i, arg := range in.TemplateArguments {
		i := i            // Capture iterator value for anonymous functions
		next := permutate // Permutation chaining
		switch arg := arg.(type) {
		case sem.FullyQualifiedName:
			switch target := arg.Target.(type) {
			case sem.TemplateParam:
				if ty, ok := s.templateTypes[target]; ok {
					args[i] = ty
				} else if num, ok := s.templateNumbers[target]; ok {
					args[i] = num
				} else {
					return nil, fmt.Errorf("'%v' was not found in templateTypes or templateNumbers", target.GetName())
				}
			default:
				perms, err := s.permutateFQN(arg)
				if err != nil {
					return nil, fmt.Errorf("while processing template argument %v: %v", i, err)
				}
				if len(perms) == 0 {
					return nil, fmt.Errorf("template argument %v has no permutations", i)
				}
				permutate = func() error {
					for _, f := range perms {
						args[i] = f
						if err := next(); err != nil {
							return err
						}
					}
					return nil
				}
			}
		default:
			return nil, fmt.Errorf("permutateFQN() unhandled template argument type: %T", arg)
		}
	}

	if err := permutate(); err != nil {
		return nil, fmt.Errorf("while processing fully qualified name '%v': %w", in.Target.GetName(), err)
	}

	return out, nil
}

func validate(fqn sem.FullyQualifiedName, uses *sem.StageUses) bool {
	if strings.HasPrefix(fqn.Target.GetName(), "_") {
		return false // Builtin, untypeable return type
	}

	isStorable := func(elTy sem.FullyQualifiedName) bool {
		elTyName := elTy.Target.GetName()
		switch {
		case elTyName == "bool",
			strings.Contains(elTyName, "sampler"),
			strings.Contains(elTyName, "texture"),
			IsAbstract(DeepestElementType(elTy)):
			return false
		}
		return true
	}

	switch fqn.Target.GetName() {
	case "array":
		if !isStorable(fqn.TemplateArguments[0].(sem.FullyQualifiedName)) {
			return false
		}
	case "ptr":
		// https://gpuweb.github.io/gpuweb/wgsl/#address-space
		access := fqn.TemplateArguments[2].(sem.FullyQualifiedName).Target.(*sem.EnumEntry).Name
		addressSpace := fqn.TemplateArguments[0].(sem.FullyQualifiedName).Target.(*sem.EnumEntry).Name
		switch addressSpace {
		case "function", "private":
			if access != "read_write" {
				return false
			}
		case "workgroup":
			uses.Vertex = false
			uses.Fragment = false
			if access != "read_write" {
				return false
			}
			if !isStorable(fqn.TemplateArguments[1].(sem.FullyQualifiedName)) {
				return false
			}
		case "uniform":
			if access != "read" {
				return false
			}
		case "storage":
			if access != "read_write" && access != "read" {
				return false
			}
		case "handle":
			if access != "read" {
				return false
			}
		default:
			return false
		}
	}

	for _, arg := range fqn.TemplateArguments {
		if argFQN, ok := arg.(sem.FullyQualifiedName); ok {
			if !validate(argFQN, uses) {
				return false
			}
		}
	}

	return true
}
