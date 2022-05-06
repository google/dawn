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
	"io"
	"reflect"
	"strings"
	"text/template"
	"unicode"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/sem"
)

type generator struct {
	s      *sem.Sem
	t      *template.Template
	cached struct {
		builtinTable *BuiltinTable // lazily built by builtinTable()
		permuter     *Permuter     // lazily built by permute()
	}
}

// WriteFile is a function that Generate() may call to emit a new file from a
// template.
// relpath is the relative path from the currently executing template.
// content is the file content to write.
type WriteFile func(relpath, content string) error

// Generate executes the template tmpl using the provided semantic
// information, writing the output to w.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func Generate(s *sem.Sem, tmpl string, w io.Writer, writeFile WriteFile) error {
	g := generator{s: s}
	return g.generate(tmpl, w, writeFile)
}

func (g *generator) generate(tmpl string, w io.Writer, writeFile WriteFile) error {
	t, err := template.New("<template>").Funcs(map[string]interface{}{
		"Map":                   newMap,
		"Iterate":               iterate,
		"Title":                 strings.Title,
		"PascalCase":            pascalCase,
		"SplitDisplayName":      splitDisplayName,
		"HasPrefix":             strings.HasPrefix,
		"HasSuffix":             strings.HasSuffix,
		"TrimPrefix":            strings.TrimPrefix,
		"TrimSuffix":            strings.TrimSuffix,
		"TrimLeft":              strings.TrimLeft,
		"TrimRight":             strings.TrimRight,
		"IsEnumEntry":           is(sem.EnumEntry{}),
		"IsEnumMatcher":         is(sem.EnumMatcher{}),
		"IsFQN":                 is(sem.FullyQualifiedName{}),
		"IsInt":                 is(1),
		"IsTemplateEnumParam":   is(sem.TemplateEnumParam{}),
		"IsTemplateNumberParam": is(sem.TemplateNumberParam{}),
		"IsTemplateTypeParam":   is(sem.TemplateTypeParam{}),
		"IsType":                is(sem.Type{}),
		"IsDeclarable":          isDeclarable,
		"IsFirstIn":             isFirstIn,
		"IsLastIn":              isLastIn,
		"BuiltinTable":          g.builtinTable,
		"Permute":               g.permute,
		"Eval":                  g.eval,
		"WriteFile":             func(relpath, content string) (string, error) { return "", writeFile(relpath, content) },
	}).Option("missingkey=error").
		Parse(tmpl)
	if err != nil {
		return err
	}
	g.t = t
	return t.Execute(w, map[string]interface{}{
		"Sem": g.s,
	})
}

// eval executes the sub-template with the given name and argument, returning
// the generated output
func (g *generator) eval(template string, args ...interface{}) (string, error) {
	target := g.t.Lookup(template)
	if target == nil {
		return "", fmt.Errorf("template '%v' not found", template)
	}
	sb := strings.Builder{}

	var err error
	if len(args) == 1 {
		err = target.Execute(&sb, args[0])
	} else {
		m := newMap()
		if len(args)%2 != 0 {
			return "", fmt.Errorf("Eval expects a single argument or list name-value pairs")
		}
		for i := 0; i < len(args); i += 2 {
			name, ok := args[i].(string)
			if !ok {
				return "", fmt.Errorf("Eval argument %v is not a string", i)
			}
			m.Put(name, args[i+1])
		}
		err = target.Execute(&sb, m)
	}

	if err != nil {
		return "", fmt.Errorf("while evaluating '%v': %v", template, err)
	}
	return sb.String(), nil
}

// builtinTable lazily calls and returns the result of buildBuiltinTable(),
// caching the result for repeated calls.
func (g *generator) builtinTable() (*BuiltinTable, error) {
	if g.cached.builtinTable == nil {
		var err error
		g.cached.builtinTable, err = buildBuiltinTable(g.s)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.builtinTable, nil
}

// permute lazily calls buildPermuter(), caching the result for repeated
// calls, then passes the argument to Permutator.Permute()
func (g *generator) permute(overload *sem.Overload) ([]Permutation, error) {
	if g.cached.permuter == nil {
		var err error
		g.cached.permuter, err = buildPermuter(g.s)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.permuter.Permute(overload)
}

// Map is a simple generic key-value map, which can be used in the template
type Map map[interface{}]interface{}

func newMap() Map { return Map{} }

// Put adds the key-value pair into the map.
// Put always returns an empty string so nothing is printed in the template.
func (m Map) Put(key, value interface{}) string {
	m[key] = value
	return ""
}

// Get looks up and returns the value with the given key. If the map does not
// contain the given key, then nil is returned.
func (m Map) Get(key interface{}) interface{} {
	return m[key]
}

// is returns a function that returns true if the value passed to the function
// matches the type of 'ty'.
func is(ty interface{}) func(interface{}) bool {
	rty := reflect.TypeOf(ty)
	return func(v interface{}) bool {
		ty := reflect.TypeOf(v)
		return ty == rty || ty == reflect.PtrTo(rty)
	}
}

// isFirstIn returns true if v is the first element of the given slice.
func isFirstIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(0).Interface() == v
}

// isFirstIn returns true if v is the last element of the given slice.
func isLastIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(count-1).Interface() == v
}

// iterate returns a slice of length 'n', with each element equal to its index.
// Useful for: {{- range Iterate $n -}}<this will be looped $n times>{{end}}
func iterate(n int) []int {
	out := make([]int, n)
	for i := range out {
		out[i] = i
	}
	return out
}

// isDeclarable returns false if the FullyQualifiedName starts with a
// leading underscore. These are undeclarable as WGSL does not allow identifers
// to have a leading underscore.
func isDeclarable(fqn sem.FullyQualifiedName) bool {
	return !strings.HasPrefix(fqn.Target.GetName(), "_")
}

// pascalCase returns the snake-case string s transformed into 'PascalCase',
// Rules:
// * The first letter of the string is capitalized
// * Characters following an underscore or number are capitalized
// * Underscores are removed from the returned string
// See: https://en.wikipedia.org/wiki/Camel_case
func pascalCase(s string) string {
	b := strings.Builder{}
	upper := true
	for _, r := range s {
		if r == '_' {
			upper = true
			continue
		}
		if upper {
			b.WriteRune(unicode.ToUpper(r))
			upper = false
		} else {
			b.WriteRune(r)
		}
		if unicode.IsNumber(r) {
			upper = true
		}
	}
	return b.String()
}

// splitDisplayName splits displayName into parts, where text wrapped in {}
// braces are not quoted and the rest is quoted. This is used to help process
// the string value of the [[display()]] decoration. For example:
//   splitDisplayName("vec{N}<{T}>")
// would return the strings:
//   [`"vec"`, `N`, `"<"`, `T`, `">"`]
func splitDisplayName(displayName string) []string {
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
