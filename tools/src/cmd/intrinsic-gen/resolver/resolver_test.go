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

package resolver_test

import (
	"fmt"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/parser"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/resolver"
)

func TestResolver(t *testing.T) {
	type test struct {
		src string
		err string
	}

	success := ""
	for _, test := range []test{
		{
			`type X`,
			success,
		}, {
			`enum E {}`,
			success,
		}, {
			`enum E {A B C}`,
			success,
		}, {
			`type X`,
			success,
		}, {
			`[[display("Y")]] type X`,
			success,
		}, {
			`
type x
match y: x`,
			success,
		}, {
			`
enum e {a b c}
match y: c | a | b`,
			success,
		}, {
			`fn f()`,
			success,
		}, {
			`fn f<T>()`,
			success,
		}, {
			`
type f32
fn f<N: num>()`,
			success,
		}, {
			`
enum e { a b c }
fn f<N: e>()`,
			success,
		}, {
			`
type f32
fn f<T>(T) -> f32`,
			success,
		}, {
			`
type f32
type P<T>
match m: f32
fn f<T: m>(P<T>) -> T`,
			success,
		}, {
			`
type f32
type P<T>
match m: f32
fn f(P<m>)`,
			success,
		}, {
			`
enum e { a }
fn f(a)`,
			success,
		}, {
			`
enum e { a b }
type T<E: e>
match m: a
fn f<E: m>(T<E>)`,
			success,
		}, {
			`
enum e { a b }
type T<E: e>
match m: a
fn f(T<m>)`,
			success,
		}, {
			`
enum e { a }
type T<E: e>
fn f(T<a>)`,
			success,
		}, {
			`
type T<E: num>
fn f<E: num>(T<E>)`,
			success,
		}, {
			`fn f<T>(T)`,
			success,
		}, {
			`
enum e { a b }
fn f<E: e>()`,
			success,
		}, {
			`
enum e { a b }
match m: a | b
fn f<E: m>()`,
			success,
		}, {
			`
type f32
type T<x>
fn f(T<T<f32>>)`,
			success,
		}, {
			`enum E {A A}`,
			`
file.txt:1:6 'A' already declared
First declared here: file.txt:1:6
`,
		},
		{
			`type X type X`,
			`
file.txt:1:13 'X' already declared
First declared here: file.txt:1:6`,
		}, {
			`[[meow]] type X`,
			`
file.txt:1:3 unknown decoration
`,
		}, {
			`[[display("Y", "Z")]] type X`,
			`
file.txt:1:3 expected a single value for 'display' decoration`,
		}, {
			`
enum e { a }
enum e { b }`,
			`
file.txt:2:6 'e' already declared
First declared here: file.txt:1:6`,
		}, {
			`
type X
match X : X`,
			`
file.txt:2:7 'X' already declared
First declared here: file.txt:1:6`,
		}, {
			`type T<X>
match M : T`,
			`file.txt:2:11 'T' requires 1 template arguments, but 0 were provided`,
		}, {
			`
match x: y`,
			`
file.txt:1:10 cannot resolve 'y'
`,
		}, {
			`
type a
match x: a | b`,
			`
file.txt:2:14 cannot resolve 'b'
`,
		}, {
			`
type a
enum e { b }
match x: a | b`,
			`
file.txt:3:14 'b' resolves to enum entry 'e.b' but type is expected
`,
		}, {
			`
type a
type b
match x: a | b | a`,
			`
file.txt:3:18 duplicate option 'a' in matcher
First declared here: file.txt:3:10
`,
		}, {
			`
enum e { a c }
match x: a | b | c`,
			`
file.txt:2:14 enum 'e' does not contain 'b'
`,
		}, {
			`
enum e { a }
match x: a
match x: a`,
			`
file.txt:3:7 'x' already declared
First declared here: file.txt:2:7
`,
		}, {
			`
type t
match x: t
match y: x`,
			`
'y' cannot be used for matcher
`,
		}, {
			`fn f(u)`,
			`file.txt:1:6 cannot resolve 'u'`,
		}, {
			`fn f() -> u`,
			`file.txt:1:11 cannot resolve 'u'`,
		}, {
			`fn f<T: u>()`,
			`file.txt:1:9 cannot resolve 'u'`,
		}, {
			`
enum e { a }
fn f() -> e`,
			`file.txt:2:11 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
fn f(T<u>)`,
			`file.txt:2:8 cannot resolve 'u'`,
		}, {
			`
type x
fn f<T>(T<x>)`,
			`file.txt:2:9 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
fn f(A<B>)`,
			`file.txt:3:8 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
fn f(A<b>)`,
			`file.txt:3:8 cannot use enum entry 'E.b' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
fn f(P<m>)`,
			`file.txt:4:8 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
fn f(P<E>)`,
			`file.txt:3:8 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: a | b
fn f(P<m>)`,
			`file.txt:4:8 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: a | b
fn f<M: m>(P<M>)`,
			`file.txt:4:14 cannot use template enum 'E' as template number`,
		}, {
			`
enum E { a }
type T<X: a>`,
			`file.txt:2:8 invalid template parameter type 'a'`,
		}, {
			`
enum E { a }
fn f<M: a>()`,
			`file.txt:2:6 invalid template parameter type 'a'`,
		},
	} {

		ast, err := parser.Parse(strings.TrimSpace(string(test.src)), "file.txt")
		if err != nil {
			t.Errorf("Unexpected parser error: %v", err)
			continue
		}

		expectErr := strings.TrimSpace(test.err)
		_, err = resolver.Resolve(ast)
		if err != nil {
			gotErr := strings.TrimSpace(fmt.Sprint(err))
			if gotErr != expectErr {
				t.Errorf("While parsing:\n%s\nGot error:\n%s\nExpected:\n%s", test.src, gotErr, expectErr)
			}
		} else if expectErr != success {
			t.Errorf("While parsing:\n%s\nGot no error, expected error:\n%s", test.src, expectErr)
		}
	}
}
