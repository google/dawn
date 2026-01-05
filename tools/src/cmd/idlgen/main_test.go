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
