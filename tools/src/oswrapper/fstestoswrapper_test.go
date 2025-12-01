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
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

// --- Path handling tests ---

func TestFSTestOSWrapper_CleanPath(t *testing.T) {
	tests := []struct {
		name     string
		path     string
		expected string
	}{
		{"Unix path", "/a/b/c", "a/b/c"},
		{"Windows path", "C:\\a\\b\\c", "C:/a/b/c"},
		{"Mixed path", "/a\\b/c", "a/b/c"},
		{"Redundant slashes", "/a//b/../c", "a/c"},
		{"Empty path", "", "."},
		{"Root path", "/", "."},
		{"Dot path", ".", "."},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateFSTestOSWrapper()
			cleaned := wrapper.CleanPath(tc.path)
			require.Equal(t, tc.expected, cleaned)
		})
	}
}

// --- FilesystemReader tests ---

func TestFSTestOSWrapper_ClosedFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		flag            int
		action          func(t *testing.T, file oswrapper.File) // Action to perform on the closed file
		expectedContent *string                                 // Expected content of the file *after* the action
		expectedError
	}{
		{
			name: "Read on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Read(make([]byte, 1))
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Write on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("fail"))
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Seek on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Seek(0, io.SeekStart)
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Stat on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Stat()
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Closed on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				err := file.Close()
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			file, err := wrapper.OpenFile(tc.path, tc.flag, 0666)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.NotNil(t, file)

			require.NoError(t, file.Close())
			if tc.action != nil {
				tc.action(t, file)
			}

			if tc.expectedContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

func TestFSTestOSWrapper_ClosedFile_MatchesReal(t *testing.T) {
	setup := matchesRealSetup{unittestSetup{
		initialFiles: map[string]string{
			"file.txt": "content",
		},
	}}
	path := "file.txt"
	flag := os.O_RDWR

	tests := []struct {
		name      string
		operation func(t *testing.T, realFile, testFile oswrapper.File)
	}{
		{
			name: "Read",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Read(make([]byte, 1))
				_, testErr := testFile.Read(make([]byte, 1))
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Write",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Write([]byte("fail"))
				_, testErr := testFile.Write([]byte("fail"))
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Seek",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Seek(0, io.SeekStart)
				_, testErr := testFile.Seek(0, io.SeekStart)
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Stat",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Stat()
				_, testErr := testFile.Stat()
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Close",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				realErr := realFile.Close()
				testErr := testFile.Close()
				requireErrorsMatch(t, realErr, testErr)
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := setup.setup(t)
			defer os.RemoveAll(realRoot)

			realFile, realOpenErr := realFS.OpenFile(filepath.Join(realRoot, path), flag, 0666)
			require.NoError(t, realOpenErr)
			require.NoError(t, realFile.Close())

			testFile, testOpenErr := testFS.OpenFile(path, flag, 0666)
			require.NoError(t, testOpenErr)
			require.NoError(t, testFile.Close())

			tc.operation(t, realFile, testFile)
			requireFileSystemsMatch(t, realRoot, testFS)
		})
	}
}

// --- FilesystemWriter tests ---

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

func TestFSTestOSWrapper_Mkdir(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name         string
		setup        unittestSetup
		path         string
		mode         os.FileMode
		expectedMode os.FileMode
		verify       func(t *testing.T, fs oswrapper.FSTestOSWrapper)
		expectedError
	}{
		{
			name:         "Create new directory",
			path:         filepath.Join(root, "newdir"),
			mode:         0755,
			expectedMode: 0755,
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				info, err := fs.Stat(filepath.Join(root, "newdir"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name:         "Create in existing subdirectory",
			path:         filepath.Join(root, "existing", "newdir"),
			mode:         0755,
			expectedMode: 0755,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "existing")},
			},
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				info, err := fs.Stat(filepath.Join(root, "existing", "newdir"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name:         "Create new directory with specific mode",
			path:         filepath.Join(root, "newdir_mode"),
			mode:         0700,
			expectedMode: 0700,
		},
		{
			name: "Directory already exists",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Path is a file",
			path: filepath.Join(root, "myfile.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "myfile.txt"): ""},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Parent directory does not exist",
			path: filepath.Join(root, "nonexistent", "newdir"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Parent path is a file",
			path: filepath.Join(root, "file.txt", "newdir"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Path is dot",
			path: ".",
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.Mkdir(tc.path, tc.mode)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.verify != nil {
				tc.verify(t, wrapper)
			}

			if tc.expectedMode != 0 {
				cleanedPath := wrapper.CleanPath(tc.path)
				file, ok := wrapper.FS[cleanedPath]
				require.True(t, ok, "file %v not found in test fs", cleanedPath)
				require.Equal(t, tc.expectedMode, file.Mode.Perm())
			}
		})
	}
}

func TestFSTestOSWrapper_Mkdir_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string // path to Mkdir
	}{
		{
			name: "Create new directory",
			path: "newdir",
		},
		{
			name: "Create in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"foo"},
			}},
			path: filepath.Join("foo", "newdir"),
		},
		{
			name: "Create in existing nested subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{filepath.Join("foo", "bar")},
			}},
			path: filepath.Join("foo", "bar", "newdir"),
		},
		{
			name: "Error on existing directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"mydir"},
			}},
			path: "mydir",
		},
		{
			name: "Error on existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"myfile": "content"},
			}},
			path: "myfile",
		},
		{
			name: "Error on non-existent parent",
			path: filepath.Join("nonexistent", "newdir"),
		},
		{
			name: "Error on parent is file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			path: filepath.Join("file.txt", "newdir"),
		},
		{
			name: "Error on dot",
			path: ".",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.Mkdir(filepath.Join(realRoot, tc.path), 0755)
			testErr := testFS.Mkdir(tc.path, 0755)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}

func TestFSTestOSWrapper_MkdirAll(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name         string
		setup        unittestSetup
		path         string
		mode         os.FileMode
		expectedMode os.FileMode
		verify       func(t *testing.T, fs oswrapper.FSTestOSWrapper)
		expectedError
	}{
		{
			name:         "Create new nested directory",
			path:         filepath.Join(root, "a", "b", "c"),
			mode:         0755,
			expectedMode: 0755,
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				for _, p := range []string{
					filepath.Join(root, "a"),
					filepath.Join(root, "a", "b"),
					filepath.Join(root, "a", "b", "c"),
				} {
					info, err := fs.Stat(p)
					require.NoError(t, err)
					require.True(t, info.IsDir())
					file, ok := fs.FS[fs.CleanPath(p)]
					require.True(t, ok)
					require.Equal(t, os.FileMode(0755), file.Mode.Perm())
				}
			},
		},
		{
			name:         "Create in existing subdirectory",
			path:         filepath.Join(root, "a", "b"),
			mode:         0755,
			expectedMode: 0755,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "a")},
			},
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				info, err := fs.Stat(filepath.Join(root, "a", "b"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name:         "Create new nested directory with specific mode",
			path:         filepath.Join(root, "d", "e", "f"),
			mode:         0700,
			expectedMode: 0700,
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				for _, p := range []string{
					filepath.Join(root, "d"),
					filepath.Join(root, "d", "e"),
					filepath.Join(root, "d", "e", "f"),
				} {
					info, err := fs.Stat(p)
					require.NoError(t, err)
					require.True(t, info.IsDir())
					file, ok := fs.FS[fs.CleanPath(p)]
					require.True(t, ok)
					require.Equal(t, os.FileMode(0700), file.Mode.Perm())
				}
			},
		},
		{
			name: "Path already exists as directory",
			path: filepath.Join(root, "a", "b"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "a", "b")},
			},
		},
		{
			name: "Part of path is a file",
			path: filepath.Join(root, "a", "b", "c"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "a", "b"): "i am a file"},
				initialDirs:  []string{filepath.Join(root, "a")},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Destination path is a file",
			path: filepath.Join(root, "a", "b"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "a", "b"): "i am a file"},
				initialDirs:  []string{filepath.Join(root, "a")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.MkdirAll(tc.path, tc.mode)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.verify != nil {
				tc.verify(t, wrapper)
			}

			if tc.expectedMode != 0 {
				cleanedPath := wrapper.CleanPath(tc.path)
				file, ok := wrapper.FS[cleanedPath]
				require.True(t, ok, "file %v not found in test fs", cleanedPath)
				require.Equal(t, tc.expectedMode, file.Mode.Perm())
			}
		})
	}
}

func TestFSTestOSWrapper_MkdirAll_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string // path to MkdirAll
	}{
		{
			name: "Create new nested directory",
			path: filepath.Join("a", "b", "c"),
		},
		{
			name: "Create in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"a"},
			}},
			path: filepath.Join("a", "b"),
		},
		{
			name: "Path already exists as directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{filepath.Join("a", "b", "c")},
			}},
			path: filepath.Join("a", "b", "c"),
		},
		{
			name: "Part of path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("a", "b"): "i am a file"},
				initialDirs:  []string{"a"},
			}},
			path: filepath.Join("a", "b", "c"),
		},
		{
			name: "Destination is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("a", "b"): "i am a file"},
				initialDirs:  []string{"a"},
			}},
			path: filepath.Join("a", "b"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.MkdirAll(filepath.Join(realRoot, tc.path), 0755)
			testErr := testFS.MkdirAll(tc.path, 0755)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}

func TestFSTestOSWrapper_MkdirTemp(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name           string
		setup          unittestSetup
		dir            string
		pattern        string
		expectedPrefix string
		expectedSuffix string
		expectedError
	}{
		{
			name:    "Simple pattern",
			dir:     filepath.Join(root, "tmp"),
			pattern: "test-",
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "tmp")},
			},
			expectedPrefix: "test-",
		},
		{
			name:    "Pattern with star",
			dir:     filepath.Join(root, "tmp"),
			pattern: "test-*-suffix",
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "tmp")},
			},
			expectedPrefix: "test-",
			expectedSuffix: "-suffix",
		},
		{
			name:    "Base dir does not exist",
			dir:     filepath.Join(root, "nonexistent"),
			pattern: "test-",
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name:    "Base dir is a file",
			dir:     filepath.Join(root, "tmpfile"),
			pattern: "test-",
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "tmpfile"): ""},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)

			gotPath, err := wrapper.MkdirTemp(tc.dir, tc.pattern)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.Equal(t, wrapper.CleanPath(tc.dir), wrapper.CleanPath(filepath.Dir(gotPath)))

			name := filepath.Base(gotPath)
			require.True(t, strings.HasPrefix(name, tc.expectedPrefix), "name '%s' should have prefix '%s'", name, tc.expectedPrefix)
			require.True(t, strings.HasSuffix(name, tc.expectedSuffix), "name '%s' should have suffix '%s'", name, tc.expectedSuffix)

			// Tests running in parallel could lead to variance in the random part of the name, so
			// just checking that is well-formed
			randomPart := strings.TrimPrefix(name, tc.expectedPrefix)
			randomPart = strings.TrimSuffix(randomPart, tc.expectedSuffix)
			_, err = strconv.ParseUint(randomPart, 10, 32)
			require.NoError(t, err, "random part '%s' is not a valid u32", randomPart)

			info, err := wrapper.Stat(gotPath)
			require.NoError(t, err, "Stat on created temp dir failed")
			require.True(t, info.IsDir(), "Created temp path is not a directory")
		})
	}
}

func TestFSTestOSWrapper_MkdirTemp_MatchesReal(t *testing.T) {
	tests := []struct {
		name    string
		setup   matchesRealSetup
		dir     string // base directory for MkdirTemp
		pattern string
	}{
		{
			name: "Simple creation",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"tmp"},
			}},
			dir:     "tmp",
			pattern: "test-",
		},
		{
			name: "Pattern with star",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"tmp"},
			}},
			dir:     "tmp",
			pattern: "test-*-suffix",
		},
		{
			name:    "Error on non-existent base dir",
			dir:     "nonexistent",
			pattern: "test-",
		},
		{
			name: "Error on base dir is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"tmpfile": ""},
			}},
			dir:     "tmpfile",
			pattern: "test-",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			realDir := filepath.Join(realRoot, tc.dir)
			testDir := tc.dir

			_, realErr := realFS.MkdirTemp(realDir, tc.pattern)
			_, testErr := testFS.MkdirTemp(testDir, tc.pattern)

			requireErrorsMatch(t, realErr, testErr)

			// Not using requireFileSystemsMatch here, because that would require FSTestOSWrapper to
			// implement the exact same behaviour as the real version. This would be difficult to
			// achieve, since the real version is intentionally designed to be hard to predict,
			// and its sources of entropy may include things like wall time or other system values.
			if realErr == nil {
				realEntries, err := os.ReadDir(realDir)
				require.NoError(t, err)
				testEntries, err := testFS.ReadDir(testDir)
				require.NoError(t, err)

				// Confirm a new directory was created for both
				require.Len(t, realEntries, 1, "expected one entry in real directory")
				require.Len(t, testEntries, 1, "expected one entry in test directory")

				realInfo, err := realEntries[0].Info()
				require.NoError(t, err)
				testInfo, err := testEntries[0].Info()
				require.NoError(t, err)

				require.True(t, realInfo.IsDir(), "real entry should be a directory")
				require.True(t, testInfo.IsDir(), "test entry should be a directory")

				// Check permissions are correct. os.MkdirTemp creates directories with mode 0700.
				require.Equal(t, os.FileMode(0700), realInfo.Mode().Perm(), "real directory permissions mismatch")
				require.Equal(t, os.FileMode(0700), testInfo.Mode().Perm(), "test directory permissions mismatch")
			}
		})
	}
}

func TestFSTestOSWrapper_Remove(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		path          string
		expectMissing []string
		expectPresent []string
		expectedError
	}{
		{
			name: "Remove file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			expectMissing: []string{filepath.Join(root, "file.txt")},
		},
		{
			name: "Remove empty directory",
			path: filepath.Join(root, "emptydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "emptydir")},
			},
			expectMissing: []string{filepath.Join(root, "emptydir")},
		},
		{
			name: "Remove file, others retained",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "file.txt"):     "content",
					filepath.Join(root, "other.txt"):    "other content",
					filepath.Join(root, "dir", "f.txt"): "in dir",
				},
				initialDirs: []string{filepath.Join(root, "otherdir")},
			},
			expectMissing: []string{filepath.Join(root, "file.txt")},
			expectPresent: []string{
				filepath.Join(root, "other.txt"),
				filepath.Join(root, "otherdir"),
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "f.txt"),
			},
		},
		{
			name: "Error on non-existent path",
			path: filepath.Join(root, "nonexistent"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Error on non-empty directory",
			path: filepath.Join(root, "nonemptydir"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "nonemptydir", "file.txt"): "content"},
				initialDirs:  []string{filepath.Join(root, "nonemptydir")},
			},
			expectPresent: []string{
				filepath.Join(root, "nonemptydir"),
				filepath.Join(root, "nonemptydir", "file.txt"),
			},
			expectedError: expectedError{
				wantErrIs: syscall.ENOTEMPTY,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.Remove(tc.path)

			for _, p := range tc.expectPresent {
				_, statErr := wrapper.Stat(p)
				require.NoError(t, statErr, "path '%s' should exist but it does not", p)
			}

			if tc.expectedError.Check(t, err) {
				return
			}

			for _, p := range tc.expectMissing {
				_, statErr := wrapper.Stat(p)
				require.Error(t, statErr, "path '%s' should not exist but it does", p)
				require.True(t, os.IsNotExist(statErr))
			}
		})
	}
}

func TestFSTestOSWrapper_Remove_MatchesReal(t *testing.T) {
	tests := []struct {
		name         string
		setup        matchesRealSetup
		pathToRemove string
	}{
		{
			name: "Remove file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			pathToRemove: "file.txt",
		},
		{
			name: "Remove file, others retained",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt":                    "content",
					"other.txt":                   "other content",
					filepath.Join("dir", "f.txt"): "in dir",
				},
				initialDirs: []string{
					"dir",
					"otherdir",
				},
			}},
			pathToRemove: "file.txt",
		},
		{
			name: "Remove empty directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"emptydir"},
			}},
			pathToRemove: "emptydir",
		},
		{
			name:         "Error on non-existent path",
			pathToRemove: "nonexistent",
		},
		{
			name: "Error on non-empty directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("dir", "file.txt"): "content"},
				initialDirs:  []string{"dir"},
			}},
			pathToRemove: "dir",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.Remove(filepath.Join(realRoot, tc.pathToRemove))
			testErr := testFS.Remove(tc.pathToRemove)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}

func TestFSTestOSWrapper_RemoveAll(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		path          string
		expectMissing []string
		expectPresent []string
		expectedError
	}{
		{
			name: "Remove single file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			expectMissing: []string{filepath.Join(root, "file.txt")},
		},
		{
			name: "Remove directory with contents",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"):           "content",
					filepath.Join(root, "dir", "subdir", "file2.txt"): "content",
				},
				initialDirs: []string{
					filepath.Join(root, "dir"),
					filepath.Join(root, "dir", "subdir"),
				},
			},
			expectMissing: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file1.txt"),
				filepath.Join(root, "dir", "subdir"),
				filepath.Join(root, "dir", "subdir", "file2.txt"),
			},
		},
		{
			name: "Remove directory with contents, others retained",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"):           "content",
					filepath.Join(root, "dir", "subdir", "file2.txt"): "content",
					filepath.Join(root, "other.txt"):                  "other content",
				},
				initialDirs: []string{
					filepath.Join(root, "dir"),
					filepath.Join(root, "dir", "subdir"),
					filepath.Join(root, "otherdir"),
				},
			},
			expectMissing: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file1.txt"),
				filepath.Join(root, "dir", "subdir"),
				filepath.Join(root, "dir", "subdir", "file2.txt"),
			},
			expectPresent: []string{
				filepath.Join(root, "other.txt"),
				filepath.Join(root, "otherdir"),
			},
		},
		{
			name: "Remove non-existent path",
			path: filepath.Join(root, "nonexistent"),
		},
		{
			name: "Path is a file, but parent is not a directory",
			path: filepath.Join(root, "a", "b"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "a"): "i am a file"},
			},
			expectPresent: []string{
				filepath.Join(root, "a"),
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.RemoveAll(tc.path)

			for _, p := range tc.expectPresent {
				_, statErr := wrapper.Stat(p)
				require.NoError(t, statErr, "path '%s' should exist but it does not", p)
			}

			if tc.expectedError.Check(t, err) {
				return
			}

			for _, p := range tc.expectMissing {
				_, statErr := wrapper.Stat(p)
				require.Error(t, statErr, "path '%s' should not exist but it does", p)
				require.True(t, os.IsNotExist(statErr))
			}
		})
	}
}

func TestFSTestOSWrapper_RemoveAll_MatchesReal(t *testing.T) {
	tests := []struct {
		name         string
		setup        matchesRealSetup
		pathToRemove string
	}{
		{
			name: "Remove single file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			pathToRemove: "file.txt",
		},
		{
			name: "Remove directory with contents",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"):           "content",
					filepath.Join("dir", "subdir", "file2.txt"): "content",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir"),
				},
			}},
			pathToRemove: "dir",
		},
		{
			name: "Remove directory with contents, others retained",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"):           "content",
					filepath.Join("dir", "subdir", "file2.txt"): "content",
					"other.txt": "other content",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir"),
					"otherdir",
				},
			}},
			pathToRemove: "dir",
		},
		{
			name:         "Remove non-existent path",
			pathToRemove: "nonexistent",
		},
		{
			name: "Path is a file, but parent is not a directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"a": "i am a file",
				},
			}},
			pathToRemove: filepath.Join("a", "b"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.RemoveAll(filepath.Join(realRoot, tc.pathToRemove))
			testErr := testFS.RemoveAll(tc.pathToRemove)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}

func TestFSTestOSWrapper_WriteFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		path          string
		content       []byte
		mode          os.FileMode
		expectContent *string
		expectedMode  os.FileMode
		expectedError
	}{
		{
			name:          "Create new file",
			path:          filepath.Join(root, "newfile.txt"),
			content:       []byte("new content"),
			mode:          0666,
			expectContent: stringPtr("new content"),
			expectedMode:  0666,
		},
		{
			name: "Overwrite existing file",
			path: filepath.Join(root, "existing.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "old content"},
			},
			content:       []byte("overwritten"),
			mode:          0666,
			expectContent: stringPtr("overwritten"),
			expectedMode:  0666,
		},
		{
			name: "Create file in existing subdirectory",
			path: filepath.Join(root, "foo", "bar.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "foo")},
			},
			content:       []byte("sub content"),
			mode:          0666,
			expectContent: stringPtr("sub content"),
			expectedMode:  0666,
		},
		{
			name:    "Error on non-existent directory",
			path:    filepath.Join(root, "new", "dir", "file.txt"),
			content: []byte("some data"),
			mode:    0666,
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Error on path is a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			content: []byte("some data"),
			mode:    0666,
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Error on parent path is a file",
			path: filepath.Join(root, "file.txt", "another.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "i am a file"},
			},
			content: []byte("some data"),
			mode:    0666,
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name:          "Create new file with specific mode",
			path:          filepath.Join(root, "newfile_mode.txt"),
			content:       []byte("new content"),
			mode:          0755,
			expectContent: stringPtr("new content"),
			expectedMode:  0755,
		},
		{
			name: "Overwrite existing file with different mode",
			path: filepath.Join(root, "existing.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "old content"},
			},
			content:       []byte("overwritten"),
			mode:          0777,
			expectContent: stringPtr("overwritten"),
			expectedMode:  0666, // Mode should be preserved
		},
		{
			name: "Write to symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "old"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			content:       []byte("new"),
			mode:          0666,
			expectContent: stringPtr("new"),
		},
		{
			name: "Write to broken symlink",
			path: filepath.Join(root, "broken_link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "broken_link"): "target.txt",
				},
			},
			content:       []byte("created"),
			mode:          0666,
			expectContent: stringPtr("created"),
		},
		{
			name: "Write to symlink to dir",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
			content: []byte("fail"),
			mode:    0666,
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Write to symlink loop",
			path: filepath.Join(root, "loop1"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "loop1"): "loop2",
					filepath.Join(root, "loop2"): "loop1",
				},
			},
			content: []byte("fail"),
			mode:    0666,
			expectedError: expectedError{
				wantErrIs: syscall.ELOOP,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.WriteFile(tc.path, tc.content, tc.mode)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.expectContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectContent, string(content))
			}

			if tc.expectedMode != 0 {
				cleanedPath := wrapper.CleanPath(tc.path)
				file, ok := wrapper.FS[cleanedPath]
				require.True(t, ok, "file %v not found in test fs", cleanedPath)
				require.Equal(t, tc.expectedMode, file.Mode.Perm())
			}
		})
	}
}

func TestFSTestOSWrapper_WriteFile_MatchesReal(t *testing.T) {
	tests := []struct {
		name    string
		setup   matchesRealSetup
		path    string
		content []byte
	}{
		{
			name:    "Create new file",
			path:    "newfile.txt",
			content: []byte("new content"),
		},
		{
			name: "Overwrite existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"existing.txt": "old content",
				},
			}},
			path:    "existing.txt",
			content: []byte("overwritten"),
		},
		{
			name: "Create file in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"foo"},
			}},
			path:    filepath.Join("foo", "bar.txt"),
			content: []byte("sub content"),
		},
		{
			name:    "Error on non-existent directory",
			path:    filepath.Join("new", "dir", "file.txt"),
			content: []byte("some data"),
		},
		{
			name: "Error on path is a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"mydir"},
			}},
			path:    "mydir",
			content: []byte("some data"),
		},
		{
			name: "Error on parent path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "i am a file",
				},
			}},
			path:    filepath.Join("file.txt", "another.txt"),
			content: []byte("some data"),
		},
		{
			name: "Write to symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "old"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path:    "link_to_file",
			content: []byte("new"),
		},
		{
			name: "Write to broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"broken_link": "target.txt",
				},
			}},
			path:    "broken_link",
			content: []byte("created"),
		},
		{
			name: "Error on symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path:    "link_to_dir",
			content: []byte("fail"),
		},
		{
			name: "Write to symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path:    "loop1",
			content: []byte("fail"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.WriteFile(filepath.Join(realRoot, tc.path), tc.content, 0666)
			testErr := testFS.WriteFile(tc.path, tc.content, 0666)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}

func TestFSTestOSWrapper_Symlink(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		oldname       string
		newname       string
		expectedError expectedError
	}{
		{
			name:    "Create valid symlink",
			oldname: "target",
			newname: filepath.Join(root, "link"),
		},
		{
			name:    "Create symlink in subdirectory",
			setup:   unittestSetup{initialDirs: []string{filepath.Join(root, "subdir")}},
			oldname: "../target",
			newname: filepath.Join(root, "subdir", "link"),
		},
		{
			name:    "Destination exists",
			setup:   unittestSetup{initialFiles: map[string]string{filepath.Join(root, "exists"): ""}},
			oldname: "target",
			newname: filepath.Join(root, "exists"),
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name:    "Parent directory does not exist",
			oldname: "target",
			newname: filepath.Join(root, "missing", "link"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name:    "Parent is not a directory",
			setup:   unittestSetup{initialFiles: map[string]string{filepath.Join(root, "file"): ""}},
			oldname: "target",
			newname: filepath.Join(root, "file", "link"),
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.Symlink(tc.oldname, tc.newname)

			if tc.expectedError.Check(t, err) {
				return
			}

			// Verify the symlink entry
			cleanedPath := wrapper.CleanPath(tc.newname)
			file, ok := wrapper.FS[cleanedPath]
			require.True(t, ok, "symlink entry not found in FS map")
			require.Equal(t, fs.ModeSymlink, file.Mode&fs.ModeSymlink)
			require.Equal(t, tc.oldname, string(file.Data))
		})
	}
}
