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

package expectations

import (
	"context"
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestUpdateExpectations(t *testing.T) {
	// Setup mock filesystem
	wrapper := oswrapper.CreateFSTestOSWrapper()

	// Determine the real Dawn root to verify fileutils.DawnRoot works with mock FS
	realWrapper := oswrapper.GetRealOSWrapper()
	rootDir := fileutils.DawnRoot(realWrapper)
	require.NotEmpty(t, rootDir, "Could not determine Dawn root")

	// Populate mock filesystem to confirm DawnRoot detection
	require.NoError(t, wrapper.MkdirAll(rootDir, 0755))
	require.NoError(t, wrapper.WriteFile(filepath.Join(rootDir, "DEPS"), []byte(""), 0666))

	// Create test list file at expected location
	testListPath := filepath.Join(rootDir, "third_party", "gn", "webgpu-cts", "test_list.txt")
	require.NoError(t, wrapper.MkdirAll(filepath.Dir(testListPath), 0755))
	require.NoError(t, wrapper.WriteFile(testListPath, []byte("webgpu:test:one\nwebgpu:test:two"), 0666))

	// Create results file
	resultsPath := filepath.Join(rootDir, "results.txt")
	resultsContent := "webgpu:test:one linux,nvidia Failure 0s false\ncore"
	require.NoError(t, wrapper.WriteFile(resultsPath, []byte(resultsContent), 0666))

	// Create expectations file
	expectationsPath := filepath.Join(rootDir, "expectations.txt")
	expectationsContent := `# BEGIN TAG HEADER
# OS
# tags: [ mac win linux android ]
# GPU
# tags: [ amd intel nvidia qualcomm ]
# Device
# tags: [ android-pixel-4 android-pixel-6 chromeos-board-amd64-generic fuchsia-board-qemu-x64 ]
# END TAG HEADER
# existing
[ mac ] webgpu:test:two: [ Failure ]
`
	require.NoError(t, wrapper.WriteFile(expectationsPath, []byte(expectationsContent), 0666))

	// Initialize command
	c := &cmd{}
	c.flags.expectations = []string{expectationsPath}
	c.flags.results.File = resultsPath
	c.flags.verbose = true

	// Run command
	ctx := context.Background()
	cfg := common.Config{OsWrapper: wrapper}

	err := c.Run(ctx, cfg)
	require.NoError(t, err)

	// Verify expectations were updated
	content, err := wrapper.ReadFile(expectationsPath)
	require.NoError(t, err)

	text := string(content)
	require.Contains(t, text, "crbug.com/0000 [ linux nvidia ] webgpu:test:one: [ Failure ]")
	require.Contains(t, text, "[ mac ] webgpu:test:two: [ Failure ]")
}
