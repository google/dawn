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

	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/parser"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/resolver"
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
			`@display("Y") type X`,
			success,
		}, {
			`
type x
match y: x`,
			success,
		}, {
			`
enum e {a b c}
match y: e.c | e.a | e.b`,
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
enum e { a }
match m: e.a
fn f(m)`,
			success,
		}, {
			`
enum e { a b }
type T<E: e>
match m: e.a
fn f<E: m>(T<E>)`,
			success,
		}, {
			`
enum e { a b }
type T<E: e>
match m: e.a
fn f(T<m>)`,
			success,
		}, {
			`
enum e { a }
type T<E: e>
match m : e.a
fn f(T<m>)`,
			success,
		}, {
			`
enum e { a }
type T<E: e>
match a : e.a
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
match m: e.a | e.b
fn f<E: m>()`,
			success,
		}, {
			`
type f32
type T<x>
fn f(T< T<f32> >)`,
			success,
		}, {
			`
type f32
op -(f32)`,
			success,
		}, {
			`
type f32
type T<x>
op +(T<f32>, T<f32>)`,
			success,
		}, {
			`
type f32
ctor f32(f32)`,
			success,
		}, {
			`
type f32
type T<x>
ctor f32(T<f32>)`,
			success,
		}, {
			`
type f32
type i32
conv f32(i32)`,
			success,
		}, {
			`
type f32
type T<x>
conv f32(T<f32>)`,
			success,
		}, {
			`
type f32
@must_use fn f() -> f32`,
			success,
		}, {
			`enum E {A A}`,
			`
file.txt:1:11 duplicate enum entry 'A'
`,
		},
		{
			`type X type X`,
			`
file.txt:1:13 'X' already declared
First declared here: file.txt:1:6`,
		}, {
			`@meow type X`,
			`
file.txt:1:2 unknown attribute
`,
		}, {
			`@display("Y", "Z") type X`,
			`
file.txt:1:2 expected a single value for 'display' attribute`,
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
type b
match x: a | b | a`,
			`
file.txt:3:18 duplicate option 'a' in matcher
First declared here: file.txt:3:10
`,
		}, {
			`
enum e { a c }
match x: e.a | e.b | e.c`,
			`
file.txt:2:18 enum 'e' does not contain 'b'
`,
		}, {
			`
enum e { a }
match x: e.a
match x: e.a`,
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
file.txt:3:10 'x' resolves to type matcher 'x' but type is expected
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
match m: E.a | E.b
fn f(P<m>)`,
			`file.txt:4:8 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
fn f<M: m>(P<M>)`,
			`file.txt:4:14 cannot use template enum 'E' as template number`,
		}, {
			`
type i
enum e { a }
op << (i) -> e`,
			`file.txt:3:14 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
op << (T<u>)`,
			`file.txt:2:10 cannot resolve 'u'`,
		}, {
			`
op << ()`,
			`file.txt:1:4 operators must have either 1 or 2 parameters`,
		}, {
			`
type i
op << (i, i, i)`,
			`file.txt:2:4 operators must have either 1 or 2 parameters`,
		}, {
			`
type x
op << <T>(T<x>)`,
			`file.txt:2:11 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
op << (A<B>)`,
			`file.txt:3:10 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
match M: E.b
op << (A<M>)`,
			`file.txt:4:10 cannot use enum matcher 'M' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
op << (P<m>)`,
			`file.txt:4:10 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
op << (P<E>)`,
			`file.txt:3:10 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
op << (P<m>)`,
			`file.txt:4:10 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
op << <M: m>(P<M>)`,
			`file.txt:4:16 cannot use template enum 'E' as template number`,
		}, {
			`
type i
enum e { a }
ctor F(i) -> e`,
			`file.txt:3:14 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
ctor F(T<u>)`,
			`file.txt:2:10 cannot resolve 'u'`,
		}, {
			`
type x
ctor F<T>(T<x>)`,
			`file.txt:2:11 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
ctor F(A<B>)`,
			`file.txt:3:10 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
match M: E.b
ctor F(A<M>)`,
			`file.txt:4:10 cannot use enum matcher 'M' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
ctor F(P<m>)`,
			`file.txt:4:10 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
ctor F(P<E>)`,
			`file.txt:3:10 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
ctor F(P<m>)`,
			`file.txt:4:10 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
ctor F<M: m>(P<M>)`,
			`file.txt:4:16 cannot use template enum 'E' as template number`,
		}, {
			`
conv F()`,
			`file.txt:1:6 conversions must have a single parameter`,
		}, {
			`
type i
conv F(i, i, i)`,
			`file.txt:2:6 conversions must have a single parameter`,
		}, {
			`
type i
enum e { a }
conv F(i) -> e`,
			`file.txt:3:14 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
conv F(T<u>)`,
			`file.txt:2:10 cannot resolve 'u'`,
		}, {
			`
type x
conv F<T>(T<x>)`,
			`file.txt:2:11 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
conv F(A<B>)`,
			`file.txt:3:10 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
match M: E.b
conv F(A<M>)`,
			`file.txt:4:10 cannot use enum matcher 'M' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
conv F(P<m>)`,
			`file.txt:4:10 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
conv F(P<E>)`,
			`file.txt:3:10 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
conv F(P<m>)`,
			`file.txt:4:10 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
conv F<M: m>(P<M>)`,
			`file.txt:4:16 cannot use template enum 'E' as template number`,
		}, {
			`
type f32
type P<T>
match m: f32
fn f(m)`,
			`file.txt:4:6 type matcher cannot be used directly here. Use a matcher constrained template argument`,
		}, {
			`
type f32
type P<T>
match m: f32
fn f(P<m>)`,
			`file.txt:4:8 type matcher cannot be used directly here. Use a matcher constrained template argument`,
		}, {
			`
@must_use fn f()`,
			`file.txt:1:2 @must_use can only be used on a function with a return type`,
		},
	} {

		ast, err := parser.Parse(strings.TrimSpace(string(test.src)), "file.txt")
		if err != nil {
			t.Errorf("While parsing:\n%s\nUnexpected parser error: %v", test.src, err)
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
