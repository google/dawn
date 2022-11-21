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

package template_test

import (
	"bytes"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/template"
	"github.com/google/go-cmp/cmp"
)

func check(t *testing.T, tmpl, expected string, fns template.Functions) {
	t.Helper()
	w := &bytes.Buffer{}
	err := template.Run(tmpl, w, fns)
	if err != nil {
		t.Errorf("template.Run() failed with %v", err)
		return
	}
	got := w.String()
	if diff := cmp.Diff(expected, got); diff != "" {
		t.Errorf("output was not as expected. Diff:\n%v", diff)
	}
}

func TestContains(t *testing.T) {
	tmpl := `
{{ Contains "hello world" "hello"}}
{{ Contains "hello world" "fish"}}
`
	expected := `
true
false
`
	check(t, tmpl, expected, nil)
}

func TestEvalSingleParameter(t *testing.T) {
	tmpl := `
pre-eval
{{ Eval "T" 123 }}
{{ Eval "T" "cat" }}
post-eval

pre-define
{{- define "T"}}
  . is {{.}}
{{- end }}
post-define
`
	expected := `
pre-eval

  . is 123

  . is cat
post-eval

pre-define
post-define
`
	check(t, tmpl, expected, nil)
}

func TestEvalParameterPairs(t *testing.T) {
	tmpl := `
pre-eval
{{ Eval "T" "number" 123 "animal" "cat" }}
post-eval

pre-define
{{- define "T"}}
  .number is {{.number}}
  .animal is {{.animal}}
{{- end }}
post-define
`
	expected := `
pre-eval

  .number is 123
  .animal is cat
post-eval

pre-define
post-define
`
	check(t, tmpl, expected, nil)
}

func TestHasPrefix(t *testing.T) {
	tmpl := `
{{ HasPrefix "hello world" "hello"}}
{{ HasPrefix "hello world" "world"}}
`
	expected := `
true
false
`
	check(t, tmpl, expected, nil)
}

func TestIterate(t *testing.T) {
	tmpl := `
{{- range $i := Iterate 5}}
  {{$i}}
{{- end}}
`
	expected := `
  0
  1
  2
  3
  4
`
	check(t, tmpl, expected, nil)
}

func TestMap(t *testing.T) {
	tmpl := `
	{{- $m := Map }}
	{{- $m.Put "one" 1 }}
	{{- $m.Put "two" 2 }}
	one: {{ $m.Get "one" }}
	two: {{ $m.Get "two" }}
`
	expected := `
	one: 1
	two: 2
`
	check(t, tmpl, expected, nil)
}

func TestPascalCase(t *testing.T) {
	tmpl := `
{{ PascalCase "hello world" }}
{{ PascalCase "hello_world" }}
`
	expected := `
HelloWorld
HelloWorld
`
	check(t, tmpl, expected, nil)
}

func TestSplit(t *testing.T) {
	tmpl := `
{{- range $i, $s := Split "cat_says_meow" "_" }}
  {{$i}}: '{{$s}}'
{{- end }}
`
	expected := `
  0: 'cat'
  1: 'says'
  2: 'meow'
`
	check(t, tmpl, expected, nil)
}

func TestTitle(t *testing.T) {
	tmpl := `
{{Title "hello world"}}
`
	expected := `
Hello World
`
	check(t, tmpl, expected, nil)
}

func TrimLeft(t *testing.T) {
	tmpl := `
'{{TrimLeft "hello world", "hel"}}'
`
	expected := `
'o world'
`
	check(t, tmpl, expected, nil)
}

func TrimPrefix(t *testing.T) {
	tmpl := `
'{{TrimLeft "hello world", "hel"}}'
'{{TrimLeft "hello world", "heo"}}'
`
	expected := `
'o world'
'hello world'
`
	check(t, tmpl, expected, nil)
}

func TrimRight(t *testing.T) {
	tmpl := `
'{{TrimRight "hello world", "wld"}}'
`
	expected := `
'hello wor'
`
	check(t, tmpl, expected, nil)
}

func TrimSuffix(t *testing.T) {
	tmpl := `
'{{TrimRight "hello world", "rld"}}'
'{{TrimRight "hello world", "wld"}}'
`
	expected := `
'hello wo'
'hello world'
`
	check(t, tmpl, expected, nil)
}
