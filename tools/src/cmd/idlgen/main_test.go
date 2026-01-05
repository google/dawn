// Copyright 2026 The Dawn & Tint Authors
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

package main

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/ben-clayton/webidlparser/ast"
	"github.com/ben-clayton/webidlparser/parser"
	"github.com/stretchr/testify/require"
)

func TestRun_MissingArgs(t *testing.T) {
	tests := []struct {
		name string
		args []string
	}{
		{
			name: "No args",
			args: []string{},
		},
		{
			name: "Missing template",
			args: []string{"--output=out.go", "input.idl"},
		},
		{
			name: "Missing input file",
			args: []string{"--template=tmpl.go", "--output=out.go"},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			wrapper := oswrapper.CreateFSTestOSWrapper()
			err := run(tt.args, wrapper)
			require.ErrorContains(t, err, "idlgen is a tool used to generate code")
		})
	}
}

func TestRun_OutputToStdout(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {}; interface GPUDevice {};"), 0644))

	args := []string{"--template=template.tmpl", "test.idl"}
	err := run(args, wrapper)
	require.NoError(t, err)
}

func TestRun_OutputToFile(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{range .Declarations}}{{.Name}} {{end}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {}; interface GPUDevice {};"), 0644))

	outputPath := "subdir/out.txt"
	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}
	err := run(args, wrapper)
	require.NoError(t, err)

	// Verify file exists and has content
	content, err := wrapper.ReadFile(outputPath)
	require.NoError(t, err)
	require.Contains(t, string(content), "Test GPUDevice")
}

func TestRun_OutputDirectoryCreationFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {};"), 0644))

	// Create a file named "subdir" to block directory creation
	require.NoError(t, wrapper.WriteFile("subdir", []byte("i am a file"), 0644))

	outputPath := "subdir/out.txt"
	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}
	err := run(args, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "failed to create output directory")
}

func TestRun_OutputFileCreationFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {};"), 0644))

	outputPath := "out.txt"
	// Create a directory at the output path to cause Create to fail
	require.NoError(t, wrapper.Mkdir(outputPath, 0755))

	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}
	err := run(args, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "failed to open output file")
}

func TestRun_TemplateReadFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	// Do not create the template file

	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {};"), 0644))

	args := []string{"--template=template.tmpl", "test.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "failed to open template file")
}

func TestRun_IDLReadFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))

	// Provide a non-existent IDL file
	args := []string{"--template=template.tmpl", "missing.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "failed to open file 'missing.idl'")
}

func TestRun_IDLParseFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	// Provide invalid IDL content
	require.NoError(t, wrapper.WriteFile("invalid.idl", []byte("this is definitely not valid idl"), 0644))

	args := []string{"--template=template.tmpl", "invalid.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "errors found while parsing invalid.idl")
}

func TestRun_TemplateParseFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	// Provide invalid template content (unclosed action)
	require.NoError(t, wrapper.WriteFile("invalid.tmpl", []byte("{{.Name"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {}; interface GPUDevice {};"), 0644))

	args := []string{"--template=invalid.tmpl", "test.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "failed to parse template file")
}

func TestRun_SuccessfulExecution(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()

	// A simple template that iterates over declarations and prints their names
	templateContent := `{{range .Declarations}}{{.Name}};{{end}}`
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte(templateContent), 0644))

	// A simple IDL file. GPUDevice is required to avoid a panic in the patch function.
	idlContent := `
		interface MyInterface {
			void myMethod();
		};
		interface GPUDevice {};
	`
	require.NoError(t, wrapper.WriteFile("test.idl", []byte(idlContent), 0644))

	outputPath := "output.txt"
	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}

	err := run(args, wrapper)
	require.NoError(t, err)

	// Verify the output content
	content, err := wrapper.ReadFile(outputPath)
	require.NoError(t, err)

	outputStr := string(content)
	require.Contains(t, outputStr, "File generated by tools/cmd/idlgen.go")
	require.Contains(t, outputStr, "\n\nMyInterface;GPUDevice;")
}

func TestSimplify(t *testing.T) {
	idl := `
		interface Dependent : Base {
			void method();
		};

		interface Base {};

		partial interface Partial {
			void method2();
		};

		interface Partial {
			void method1();
		};

		interface mixin MyMixin {
			const long mixinConst = 1;
		};

		interface WithMixin {
			void originalMethod();
		};

		WithMixin includes MyMixin;
	`

	parsed := parser.Parse(idl)
	require.Empty(t, parsed.Errors)

	simplified, decls := simplify(parsed)

	// Check Partial Interface Merging
	partialDecl := decls["Partial"]
	require.NotNil(t, partialDecl)
	partialIface, ok := partialDecl.(*ast.Interface)
	require.True(t, ok)
	require.Equal(t, 2, len(partialIface.Members))

	memberNames := make(map[string]bool)
	for _, m := range partialIface.Members {
		if member, ok := m.(*ast.Member); ok {
			memberNames[member.Name] = true
		}
	}
	require.True(t, memberNames["method1"])
	require.True(t, memberNames["method2"])

	// Check Mixin Embedding
	withMixinDecl := decls["WithMixin"]
	require.NotNil(t, withMixinDecl)
	withMixinIface, ok := withMixinDecl.(*ast.Interface)
	require.True(t, ok)
	require.Equal(t, 2, len(withMixinIface.Members))

	memberNames = make(map[string]bool)
	for _, m := range withMixinIface.Members {
		if member, ok := m.(*ast.Member); ok {
			memberNames[member.Name] = true
		}
	}
	require.True(t, memberNames["originalMethod"])
	require.True(t, memberNames["mixinConst"])

	// Check Dependency Ordering
	// Base must precede Dependent
	var names []string
	for _, d := range simplified.Declarations {
		names = append(names, nameOf(d))
	}

	baseIdx := -1
	dependentIdx := -1
	for i, name := range names {
		if name == "Base" {
			baseIdx = i
		}
		if name == "Dependent" {
			dependentIdx = i
		}
	}
	require.NotEqual(t, -1, baseIdx, "Base not found in simplified declarations")
	require.NotEqual(t, -1, dependentIdx, "Dependent not found in simplified declarations")
	require.True(t, baseIdx < dependentIdx, "Base should precede Dependent")
}

func TestEnumEntryName(t *testing.T) {
	tests := []struct {
		input    string
		expected string
	}{
		{"rgba8unorm", "kRgba8Unorm"},
		{"\"bgra8unorm\"", "kBgra8Unorm"},
		{"depth-stencil", "kDepthStencil"},
		{"triangle-list", "kTriangleList"},
		{"line-strip", "kLineStrip"},
	}

	for _, tt := range tests {
		t.Run(tt.input, func(t *testing.T) {
			result := enumEntryName(tt.input)
			require.Equal(t, result, tt.expected)
		})
	}
}

func TestMemberExtraction(t *testing.T) {
	idl := `
		interface Base {
			const long BaseConst = 1;
			attribute long BaseAttr;
			void baseMethod();
		};

		interface Derived : Base {
			const long DerivedConst = 2;
			attribute long DerivedAttr;
			void derivedMethod();
		};

		namespace MyNamespace {
			const long NamespaceConst = 3;
			attribute long NamespaceAttr;
			void namespaceMethod();
		};
	`

	parsed := parser.Parse(idl)
	require.Empty(t, parsed.Errors)

	_, decls := simplify(parsed)

	g := generator{
		declarations: decls,
	}

	base := decls["Base"]
	derived := decls["Derived"]
	namespace := decls["MyNamespace"]

	t.Run("constantsOf", func(t *testing.T) {
		check := func(obj ast.Decl, expected []string) {
			consts := constantsOf(obj)
			require.Len(t, consts, len(expected))
			for i, name := range expected {
				require.Equal(t, name, consts[i].Name)
			}
		}
		check(base, []string{"BaseConst"})
		check(derived, []string{"DerivedConst"})
		check(namespace, []string{"NamespaceConst"})
	})

	t.Run("flattenedConstantsOf", func(t *testing.T) {
		consts := g.flattenedConstantsOf(derived)
		require.Len(t, consts, 2)
		require.Equal(t, "DerivedConst", consts[0].Name)
		require.Equal(t, "BaseConst", consts[1].Name)
	})

	t.Run("attributesOf", func(t *testing.T) {
		check := func(obj ast.Decl, expected []string) {
			attrs := attributesOf(obj)
			require.Len(t, attrs, len(expected))
			for i, name := range expected {
				require.Equal(t, name, attrs[i].Name)
			}
		}
		check(base, []string{"BaseAttr"})
		check(derived, []string{"DerivedAttr"})
		check(namespace, []string{"NamespaceAttr"})
	})

	t.Run("flattenedAttributesOf", func(t *testing.T) {
		attrs := g.flattenedAttributesOf(derived)
		require.Len(t, attrs, 2)
		require.Equal(t, "DerivedAttr", attrs[0].Name)
		require.Equal(t, "BaseAttr", attrs[1].Name)
	})

	t.Run("methodsOf", func(t *testing.T) {
		check := func(obj ast.Decl, expected []string) {
			methods := methodsOf(obj)
			require.Len(t, methods, len(expected))
			for i, name := range expected {
				require.Equal(t, name, methods[i].Name)
			}
		}
		check(base, []string{"baseMethod"})
		check(derived, []string{"derivedMethod"})
		// methodsOf currently returns nil for namespaces
		check(namespace, []string{})
	})

	t.Run("flattenedMethodsOf", func(t *testing.T) {
		methods := g.flattenedMethodsOf(derived)
		require.Len(t, methods, 2)
		require.Equal(t, "derivedMethod", methods[0].Name)
		require.Equal(t, "baseMethod", methods[1].Name)
	})
}

func TestPascalCase(t *testing.T) {
	tests := []struct {
		input    string
		expected string
	}{
		{"foo_bar", "FooBar"},
		{"gl-Position", "GlPosition"},
		{"texture_2d", "Texture2D"},
		{"simple", "Simple"},
		{"camelCase", "CamelCase"},
		{"_leading_underscore", "LeadingUnderscore"},
		{"trailing_underscore_", "TrailingUnderscore"},
		{"multiple___underscores", "MultipleUnderscores"},
		{"123_numbers", "123Numbers"},
		{"mixed-separators_test", "MixedSeparatorsTest"},
	}

	for _, tt := range tests {
		t.Run(tt.input, func(t *testing.T) {
			result := pascalCase(tt.input)
			require.Equal(t, result, tt.expected)
		})
	}
}
