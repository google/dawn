// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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
package oswrapper_test

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/require"
)

// Tests for the Create() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_Create(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		contentToWrite  []byte
		expectedContent *string
		expectedError
	}{
		{
			name:            "Create new file and write",
			path:            filepath.Join(root, "newfile.txt"),
			contentToWrite:  []byte("hello"),
			expectedContent: stringPtr("hello"),
		},
		{
			name: "Truncate existing file",
			path: filepath.Join(root, "existing.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "old content"},
			},
			contentToWrite:  []byte("new"),
			expectedContent: stringPtr("new"),
		},
		{
			name: "Create file in existing subdirectory",
			path: filepath.Join(root, "foo", "bar.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "foo")},
			},
			contentToWrite:  []byte("sub content"),
			expectedContent: stringPtr("sub content"),
		},
		{
			name: "Create file in non-existent directory",
			path: filepath.Join(root, "new", "dir", "file.txt"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Path is a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Parent path is a file",
			path: filepath.Join(root, "file.txt", "another.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "i am a file"},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			file, err := wrapper.Create(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			// For success cases:
			require.NotNil(t, file)
			defer file.Close()

			if len(tc.contentToWrite) > 0 {
				n, err := file.Write(tc.contentToWrite)
				require.NoError(t, err)
				require.Equal(t, len(tc.contentToWrite), n)
			}

			// Close the file to ensure contents are flushed to the in-memory map.
			require.NoError(t, file.Close())

			if tc.expectedContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

// TestFSTestOSWrapper_Create_MatchesReal tests that the behavior of FSTestOSWrapper.Create
// matches the behavior of the real os.Create function.
func TestFSTestOSWrapper_Create_MatchesReal(t *testing.T) {
	tests := []struct {
		name           string
		setup          matchesRealSetup
		path           string // path to Create
		contentToWrite []byte
	}{
		{
			name:           "Create new file",
			path:           "newfile.txt",
			contentToWrite: []byte("hello"),
		},
		{
			name: "Truncate existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"existing.txt": "old content",
				},
			}},
			path:           "existing.txt",
			contentToWrite: []byte("new"),
		},
		{
			name: "Create file in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"foo",
				},
			}},
			path:           filepath.Join("foo", "bar.txt"),
			contentToWrite: []byte("sub content"),
		},
		{
			name: "Error on non-existent directory",
			path: filepath.Join("new", "dir", "file.txt"),
		},
		{
			name: "Error on path is a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"mydir",
				},
			}},
			path: "mydir",
		},
		{
			name: "Error on parent path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "i am a file",
				},
			}},
			path: filepath.Join("file.txt", "another.txt"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			realPath := filepath.Join(realRoot, tc.path)
			realFile, realErr := realFS.Create(realPath)
			if realErr == nil {
				if len(tc.contentToWrite) > 0 {
					_, err := realFile.Write(tc.contentToWrite)
					require.NoError(t, err)
				}
				require.NoError(t, realFile.Close())
			}

			testFile, testErr := testFS.Create(tc.path)
			if testErr == nil {
				if len(tc.contentToWrite) > 0 {
					_, err := testFile.Write(tc.contentToWrite)
					require.NoError(t, err)
				}
				require.NoError(t, testFile.Close())
			}

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
