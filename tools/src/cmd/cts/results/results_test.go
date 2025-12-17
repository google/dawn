// Copyright 2025 The Dawn & Tint Authors
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

package results

import (
	"context"
	"io"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func createConfigAndOsWrapper() (common.Config, *oswrapper.FSTestOSWrapper) {
	w := oswrapper.CreateFSTestOSWrapper()
	cfg := common.Config{
		OsWrapper: w,
	}
	return cfg, &w
}

func createResultsFile(t *testing.T, w oswrapper.OSWrapper, path string, content string) {
	f, err := w.Create(path)
	require.NoError(t, err)
	defer f.Close()
	_, err = f.Write([]byte(content))
	require.NoError(t, err)
}

func TestRun_OutputToStdout(t *testing.T) {
	ctx := context.Background()
	cfg, w := createConfigAndOsWrapper()

	inputPath := "input_results.txt"
	inputContent := "test/query Pass 1s false\ncore\n"
	createResultsFile(t, w, inputPath, inputContent)

	c := &cmd{}
	c.flags.source.File = inputPath
	c.flags.output = "-"

	// No way to check stdout content without additional dependency injection,
	// so just check that there is no error.
	err := c.Run(ctx, cfg)
	require.NoError(t, err)
}

func TestRun_OutputToFile(t *testing.T) {
	ctx := context.Background()
	cfg, w := createConfigAndOsWrapper()

	inputPath := "input_results.txt"
	inputContent := "test/query Pass 1s false\ncore\n"
	createResultsFile(t, w, inputPath, inputContent)

	outputPath := "output_results.txt"

	c := &cmd{}
	c.flags.source.File = inputPath
	c.flags.output = outputPath

	err := c.Run(ctx, cfg)
	require.NoError(t, err)

	f, err := w.Open(outputPath)
	require.NoError(t, err)
	defer f.Close()
	content, err := io.ReadAll(f)
	require.NoError(t, err)
	require.Equal(t, inputContent, string(content))
}

func TestRun_UnsuppressedFailuresOnly(t *testing.T) {
	ctx := context.Background()
	cfg, w := createConfigAndOsWrapper()

	inputPath := "input_results.txt"
	inputContent := "test/query Pass 1s false\ncore\n"
	createResultsFile(t, w, inputPath, inputContent)

	outputPath := "output_results.txt"

	c := &cmd{}
	c.flags.source.File = inputPath
	c.flags.output = outputPath
	c.flags.unsuppressedFailuresOnly = true

	err := c.Run(ctx, cfg)
	require.NoError(t, err)

	f, err := w.Open(outputPath)
	require.NoError(t, err)
	defer f.Close()
	content, err := io.ReadAll(f)
	require.NoError(t, err)
	require.Equal(t, inputContent, string(content))
}

func TestRun_InputFileNotFound(t *testing.T) {
	ctx := context.Background()
	cfg, _ := createConfigAndOsWrapper()

	c := &cmd{}
	c.flags.source.File = "nonexistent.txt"
	c.flags.output = "-"

	err := c.Run(ctx, cfg)
	require.ErrorContains(t, err, "file does not exist")
}

func TestRun_OutputFileCreationError(t *testing.T) {
	ctx := context.Background()
	cfg, w := createConfigAndOsWrapper()

	inputPath := "input_results.txt"
	inputContent := "test/query Pass 1s false\ncore\n"
	createResultsFile(t, w, inputPath, inputContent)

	outputPath := "output_dir"
	// Create a directory at outputPath to force file creation failure.
	require.NoError(t, w.Mkdir(outputPath, 0755))

	c := &cmd{}
	c.flags.source.File = inputPath
	c.flags.output = outputPath

	err := c.Run(ctx, cfg)
	require.Error(t, err)
	require.ErrorContains(t, err, "failed to open output file")
}
