// Copyright 2022 The Tint Authors.
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

// package template wraps the golang "text/template" package to provide an
// enhanced template generator.
package template

import (
	"fmt"
	"io"
	"io/ioutil"
	"path/filepath"
	"strings"
	"text/template"
	"unicode"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// The template function binding table
type Functions map[string]interface{}

// Run executes the template tmpl, writing the output to w.
// funcs are the functions provided to the template.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func Run(tmpl string, w io.Writer, funcs Functions) error {
	g := generator{
		template: template.New("<template>"),
	}

	globals := newMap()

	// Add a bunch of generic useful functions
	g.funcs = Functions{
		"Contains":   strings.Contains,
		"Eval":       g.eval,
		"Globals":    func() Map { return globals },
		"HasPrefix":  strings.HasPrefix,
		"HasSuffix":  strings.HasSuffix,
		"Import":     g.importTmpl,
		"Iterate":    iterate,
		"Map":        newMap,
		"PascalCase": pascalCase,
		"Split":      strings.Split,
		"Title":      strings.Title,
		"TrimLeft":   strings.TrimLeft,
		"TrimPrefix": strings.TrimPrefix,
		"TrimRight":  strings.TrimRight,
		"TrimSuffix": strings.TrimSuffix,
	}

	// Append custom functions
	for name, fn := range funcs {
		g.funcs[name] = fn
	}

	if err := g.bindAndParse(g.template, tmpl); err != nil {
		return err
	}

	return g.template.Execute(w, nil)
}

type generator struct {
	template *template.Template
	funcs    Functions
}

func (g *generator) bindAndParse(t *template.Template, tmpl string) error {
	_, err := t.
		Funcs(map[string]interface{}(g.funcs)).
		Option("missingkey=error").
		Parse(tmpl)
	return err
}

// eval executes the sub-template with the given name and argument, returning
// the generated output
func (g *generator) eval(template string, args ...interface{}) (string, error) {
	target := g.template.Lookup(template)
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

// importTmpl parses the template at the given project-relative path, merging
// the template definitions into the global namespace.
// Note: The body of the template is not executed.
func (g *generator) importTmpl(path string) (string, error) {
	if strings.Contains(path, "..") {
		return "", fmt.Errorf("import path must not contain '..'")
	}
	path = filepath.Join(fileutils.DawnRoot(), path)
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("failed to open '%v': %w", path, err)
	}
	t := g.template.New("")
	if err := g.bindAndParse(t, string(data)); err != nil {
		return "", fmt.Errorf("failed to parse '%v': %w", path, err)
	}
	if err := t.Execute(ioutil.Discard, nil); err != nil {
		return "", fmt.Errorf("failed to execute '%v': %w", path, err)
	}
	return "", nil
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

// iterate returns a slice of length 'n', with each element equal to its index.
// Useful for: {{- range Iterate $n -}}<this will be looped $n times>{{end}}
func iterate(n int) []int {
	out := make([]int, n)
	for i := range out {
		out[i] = i
	}
	return out
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
		if r == '_' || r == ' ' {
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
