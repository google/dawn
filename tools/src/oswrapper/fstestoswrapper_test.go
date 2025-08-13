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
	"errors"
	"io"
	"os"
	"path/filepath"
	"runtime"
	"syscall"
	"testing"
	"testing/fstest"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

// --- Generic helpers ---

// stringPtr is a helper to get a pointer to a string, used so test cases can have nil to indicate a
// string value isn't set, instead of testing for "".
func stringPtr(s string) *string {
	return &s
}

// getTestRoot returns the standard root path for the current OS.
func getTestRoot() string {
	if runtime.GOOS == "windows" {
		return "C:\\"
	}
	return "/"
}

// --- Unittest specific helpers ---

// unittestSetup is a helper for setting up the FSTestOSWrapper for an unittest.
type unittestSetup struct {
	initialFiles map[string]string
	initialDirs  []string
}

// setup initializes the FSTestOSWrapper with the files and directories specified
// in the unittestSetup struct.
func (s unittestSetup) setup(t *testing.T) oswrapper.FSTestOSWrapper {
	t.Helper()
	testFS := oswrapper.CreateFSTestOSWrapper()

	for path, content := range s.initialFiles {
		require.NoError(t, testFS.MkdirAll(filepath.Dir(path), 0755))
		require.NoError(t, testFS.WriteFile(path, []byte(content), 0666))
	}
	for _, path := range s.initialDirs {
		require.NoError(t, testFS.MkdirAll(path, 0755))
	}
	return testFS
}

// expectedError is a helper struct for testing error conditions.
// Note: This is meant for usage in the non-*_MatchesReal unittests.
type expectedError struct {
	wantErrIs  error
	wantErrMsg string
}

// Check verifies that the given error `err` matches the expected error conditions.
// It returns true if an error was expected and found, indicating the test should stop.
func (e expectedError) Check(t *testing.T, err error) (stopTest bool) {
	t.Helper()
	if e.wantErrIs == nil && e.wantErrMsg == "" {
		require.NoError(t, err)
		return false
	}

	require.Error(t, err)
	if e.wantErrIs != nil {
		require.ErrorIs(t, err, e.wantErrIs)
	}
	if e.wantErrMsg != "" {
		require.ErrorContains(t, err, e.wantErrMsg)
	}
	return true
}

// --- *_MatchesReal specific helpers ---

// matchesRealSetup is a helper for setting up the filesystem for a test case that
// matches against the real OS.
// Note: This is meant for usage in the *_MatchesReal tests.
type matchesRealSetup struct {
	unittestSetup
}

// setup creates a temporary directory for the real OS, and initializes both the
// real and test filesystems with the files and directories specified in the
// matchesRealSetup struct.
func (s matchesRealSetup) setup(t *testing.T) (string, oswrapper.OSWrapper, oswrapper.FSTestOSWrapper) {
	t.Helper()

	// Setup the test wrapper using the embedded setup method.
	testFS := s.unittestSetup.setup(t)

	// Setup the real FS
	realRoot, err := os.MkdirTemp("", "real_fs_test_*")
	require.NoError(t, err)
	realFS := oswrapper.GetRealOSWrapper()

	for path, content := range s.initialFiles {
		realPath := filepath.Join(realRoot, path)
		require.NoError(t, os.MkdirAll(filepath.Dir(realPath), 0755))
		require.NoError(t, os.WriteFile(realPath, []byte(content), 0666))
	}
	for _, path := range s.initialDirs {
		require.NoError(t, os.MkdirAll(filepath.Join(realRoot, path), 0755))
	}

	return realRoot, realFS, testFS
}

// requireFileSystemsMatch walks the real filesystem at realRoot and compares its
// structure and file content against the provided FSTestOSWrapper.
// Note: This is meant for usage in *_MatchesReal tests.
func requireFileSystemsMatch(t *testing.T, realRoot string, testFS oswrapper.FSTestOSWrapper) {
	t.Helper()

	realMap := make(map[string]*fstest.MapFile)
	err := filepath.Walk(realRoot, func(path string, info os.FileInfo, err error) error {
		require.NoError(t, err)
		if path == realRoot {
			return nil
		}

		relPath, err := filepath.Rel(realRoot, path)
		require.NoError(t, err)

		// Use the same path cleaning logic as FSTestOSWrapper.
		mapKey := testFS.FSTestFilesystemReaderWriter.CleanPath(relPath)

		mapFile := &fstest.MapFile{Mode: info.Mode()}
		if !info.IsDir() {
			data, err := os.ReadFile(path)
			require.NoError(t, err)
			mapFile.Data = data
		}
		realMap[mapKey] = mapFile
		return nil
	})
	require.NoError(t, err)

	testMap := testFS.FS
	require.Equal(t, len(realMap), len(testMap), "Filesystems have a different number of entries")

	for key, realFile := range realMap {
		testFile, ok := testMap[key]
		require.True(t, ok, "Path '%s' exists in real FS but not in FS under test", key)

		require.Equal(t, realFile.Mode.IsDir(), testFile.Mode.IsDir(), "IsDir mismatch for '%s'", key)

		if !realFile.Mode.IsDir() {
			require.Equal(t, realFile.Data, testFile.Data, "Content mismatch for file '%s'", key)
		}
	}
}

// requireErrorsMatch asserts that the real and test errors are compatible.
// Both can be nil. If the real error is a well-known os error, the test error
// must be the same. Otherwise, it just checks that both are non-nil.
// Note: This is meant for usage in *_MatchesReal tests.
func requireErrorsMatch(t *testing.T, realErr, testErr error) {
	t.Helper()
	if realErr != nil {
		require.Error(t, testErr, "Real FS errored but FS under test did not.\nReal error: %v", realErr)

		// For certain well-defined errors, we expect the test wrapper to return
		// the exact same error type.
		if errors.Is(realErr, os.ErrNotExist) {
			require.ErrorIs(t, testErr, os.ErrNotExist, "Real error is os.ErrNotExist, but test error is not")
		} else if errors.Is(realErr, os.ErrExist) {
			require.ErrorIs(t, testErr, os.ErrExist, "Real error is os.ErrExist, but test error is not")
		} else if errors.Is(realErr, syscall.ENOTEMPTY) {
			require.ErrorIs(t, testErr, syscall.ENOTEMPTY, "Real error is syscall.ENOTEMPTY, but test error is not")
		} else if errors.Is(realErr, os.ErrClosed) {
			require.ErrorIs(t, testErr, os.ErrClosed, "Real error is os.ErrClosed, but test error is not")
		}
		// For other errors (e.g., 'is a directory', 'directory not empty'),
		// the exact error message can be OS-dependent. In these cases, just
		// checking that *an* error occurred is sufficient.
	} else {
		require.NoError(t, testErr, "FS under test errored but Real FS did not.\nTest error: %v", testErr)
	}
}

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

func TestFSTestOSWrapper_Open(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		expectedContent *string
		expectedError
	}{
		{
			name: "Open existing file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "file.txt"): "hello world",
				},
			},
			expectedContent: stringPtr("hello world"),
		},
		{
			name: "Open non-existent file",
			path: filepath.Join(root, "nonexistent.txt"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Open a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
		},
		{
			name: "Parent path is a file",
			path: filepath.Join(root, "file.txt", "another.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "file.txt"): "i am a file",
				},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			file, err := wrapper.Open(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.NotNil(t, file)
			defer file.Close()

			if tc.expectedContent != nil {
				content, err := io.ReadAll(file)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

func TestFSTestOSWrapper_Open_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string // path to Open
	}{
		{
			name: "Open existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "hello world",
				},
			}},
			path: "file.txt",
		},
		{
			name: "Error on non-existent file",
			path: "nonexistent.txt",
		},
		{
			name: "Open a directory",
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

			realFile, realErr := realFS.Open(filepath.Join(realRoot, tc.path))
			if realErr == nil {
				defer realFile.Close()
			}
			testFile, testErr := testFS.Open(tc.path)
			if testErr == nil {
				defer testFile.Close()
			}

			requireErrorsMatch(t, realErr, testErr)

			if realErr == nil {
				realInfo, err := realFile.Stat()
				require.NoError(t, err)
				testInfo, err := testFile.Stat()
				require.NoError(t, err)

				require.Equal(t, realInfo.IsDir(), testInfo.IsDir(), "IsDir mismatch for opened path")

				if !realInfo.IsDir() {
					realContent, err := io.ReadAll(realFile)
					require.NoError(t, err)
					testContent, err := io.ReadAll(testFile)
					require.NoError(t, err)
					require.Equal(t, realContent, testContent)
				}
			}
		})
	}
}

func TestFSTestOSWrapper_OpenFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		flag            int
		action          func(t *testing.T, file oswrapper.File) // Action to perform on the opened file
		expectedContent *string                                 // Expected content of the file *after* the action
		expectedError
	}{
		{
			name: "O_RDONLY - Read existing file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "read me"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				content, err := io.ReadAll(file)
				require.NoError(t, err)
				require.Equal(t, "read me", string(content))
			},
		},
		{
			name: "O_RDONLY - Error on write",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "read only"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("fail"))
				require.Error(t, err)
			},
			expectedContent: stringPtr("read only"),
		},
		{
			name: "O_WRONLY - Write to existing file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "initial"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("newtial"),
		},
		{
			name: "O_WRONLY - Error on read",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "write only"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := io.ReadAll(file)
				require.Error(t, err)
			},
			expectedContent: stringPtr("write only"),
		},
		{
			name: "O_RDWR - Read and Write",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "start"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				// This should not advance the write offset
				content, err := io.ReadAll(file)
				require.NoError(t, err)
				require.Equal(t, "start", string(content))

				// Seek to the beginning to overwrite
				_, err = file.Seek(0, io.SeekStart)
				require.NoError(t, err)

				_, err = file.Write([]byte("finish"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("finish"),
		},
		{
			name: "O_APPEND - Append to file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY | os.O_APPEND,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "first,"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("second"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("first,second"),
		},
		{
			name: "O_CREATE - Create new file",
			path: filepath.Join(root, "newfile.txt"),
			flag: os.O_WRONLY | os.O_CREATE,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("created"),
		},
		{
			name: "O_CREATE | O_EXCL - Fail if exists",
			path: filepath.Join(root, "existing.txt"),
			flag: os.O_WRONLY | os.O_CREATE | os.O_EXCL,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "..."},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "O_TRUNC - Truncate existing file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY | os.O_TRUNC,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "to be truncated"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new data"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("new data"),
		},
		{
			name: "Error - Open non-existent for reading",
			path: filepath.Join(root, "no.txt"),
			flag: os.O_RDONLY,
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Error - Path is a directory",
			path: filepath.Join(root, "mydir"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
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
			defer file.Close()
			if tc.action != nil {
				tc.action(t, file)
			}

			// Close the file to ensure contents are flushed.
			require.NoError(t, file.Close())

			if tc.expectedContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

func TestFSTestOSWrapper_OpenFile_MatchesReal(t *testing.T) {
	tests := []struct {
		name   string
		setup  matchesRealSetup
		path   string
		flag   int
		action func(t *testing.T, file oswrapper.File)
	}{
		{
			name: "O_RDONLY - Read existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "read me",
				},
			}},
			path: "file.txt",
			flag: os.O_RDONLY,
		},
		{
			name: "O_RDONLY - Error on write",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "read only",
				},
			}},
			path: "file.txt",
			flag: os.O_RDONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("fail"))
				require.Error(t, err)
			},
		},
		{
			name: "O_WRONLY - Write to existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "initial",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_WRONLY - Error on read",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "write only",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := io.ReadAll(file)
				require.Error(t, err)
			},
		},
		{
			name: "O_RDWR - Read and Write",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "start",
				},
			}},
			path: "file.txt",
			flag: os.O_RDWR,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("finish"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_APPEND - Append to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "first,",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY | os.O_APPEND,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("second"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_CREATE - Create new file",
			path: "newfile.txt",
			flag: os.O_WRONLY | os.O_CREATE,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_CREATE | O_EXCL - Fail if exists",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"existing.txt": "...",
				},
			}},
			path: "existing.txt",
			flag: os.O_WRONLY | os.O_CREATE | os.O_EXCL,
		},
		{
			name: "O_TRUNC - Truncate existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "to be truncated",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY | os.O_TRUNC,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new data"))
				require.NoError(t, err)
			},
		},
		{
			name: "Error - Open non-existent for reading",
			path: "no.txt",
			flag: os.O_RDONLY,
		},
		{
			name: "Error - Path is a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"mydir"},
			}},
			path: "mydir",
			flag: os.O_WRONLY,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute on Real FS
			realFile, realErr := realFS.OpenFile(filepath.Join(realRoot, tc.path), tc.flag, 0666)
			if realErr == nil {
				defer realFile.Close()
				if tc.action != nil {
					tc.action(t, realFile)
				}
				require.NoError(t, realFile.Close())
			}

			// Execute on Test FS
			testFile, testErr := testFS.OpenFile(tc.path, tc.flag, 0666)
			if testErr == nil {
				defer testFile.Close()
				if tc.action != nil {
					tc.action(t, testFile)
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

func TestFSTestOSWrapper_ReadFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		expectedContent []byte
		expectedError
	}{
		{
			name: "Read existing file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "hello world"},
			},
			expectedContent: []byte("hello world"),
		},
		{
			name: "Read non-existent file",
			path: filepath.Join(root, "nonexistent.txt"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Read a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			content, err := wrapper.ReadFile(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.Equal(t, tc.expectedContent, content)
		})
	}
}

func TestFSTestOSWrapper_ReadFile_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string
	}{
		{
			name: "Read existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "hello world",
				},
			}},
			path: "file.txt",
		},
		{
			name: "Error on non-existent file",
			path: "nonexistent.txt",
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
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realContent, realErr := realFS.ReadFile(filepath.Join(realRoot, tc.path))
			testContent, testErr := testFS.ReadFile(tc.path)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				require.Equal(t, realContent, testContent)
			}
		})
	}
}

func TestFSTestOSWrapper_ReadDir(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		expectedEntries []string
		expectedError
	}{
		{
			name: "Read empty directory",
			path: filepath.Join(root, "emptydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "emptydir")},
			},
			expectedEntries: []string{},
		},
		{
			name: "Read directory with files and subdirectories",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"):  "",
					filepath.Join(root, "dir", "z_file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir", "subdir", "nested")},
			},
			expectedEntries: []string{"file1.txt", "subdir", "z_file.txt"},
		},
		{
			name: "Read non-existent directory",
			path: filepath.Join(root, "nonexistent"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Read a file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			entries, err := wrapper.ReadDir(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			entryNames := make([]string, len(entries))
			for i, e := range entries {
				entryNames[i] = e.Name()
			}
			require.ElementsMatch(t, tc.expectedEntries, entryNames)
		})
	}
}

func TestFSTestOSWrapper_ReadDir_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string
	}{
		{
			name: "Read empty directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"emptydir"},
			}},
			path: "emptydir",
		},
		{
			name: "Read directory with files and subdirectories",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"):  "content",
					filepath.Join("dir", "z_file.txt"): "content",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir", "nested"),
				},
			}},
			path: "dir",
		},
		{
			name: "Error on non-existent directory",
			path: "nonexistent",
		},
		{
			name: "Error on path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			path: "file.txt",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realEntries, realErr := realFS.ReadDir(filepath.Join(realRoot, tc.path))
			testEntries, testErr := testFS.ReadDir(tc.path)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				realNames := make([]string, len(realEntries))
				for i, e := range realEntries {
					realNames[i] = e.Name()
				}
				testNames := make([]string, len(testEntries))
				for i, e := range testEntries {
					testNames[i] = e.Name()
				}
				require.ElementsMatch(t, realNames, testNames)
			}
		})
	}
}

func TestFSTestOSWrapper_Stat(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name   string
		setup  unittestSetup
		path   string
		verify func(t *testing.T, info os.FileInfo)
		expectedError
	}{
		{
			name: "Stat a file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "file.txt", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
		{
			name: "Stat a directory",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.True(t, info.IsDir())
				require.Equal(t, "dir", info.Name())
			},
		},
		{
			name: "Stat non-existent path",
			path: filepath.Join(root, "nonexistent"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			info, err := wrapper.Stat(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.verify != nil {
				tc.verify(t, info)
			}
		})
	}
}

func TestFSTestOSWrapper_Stat_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string
	}{
		{
			name: "Stat a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "content",
				},
			}},
			path: "file.txt",
		},
		{
			name: "Stat a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"dir",
				},
			}},
			path: "dir",
		},
		{
			name: "Stat non-existent path",
			path: "nonexistent",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realInfo, realErr := realFS.Stat(filepath.Join(realRoot, tc.path))
			testInfo, testErr := testFS.Stat(tc.path)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				require.Equal(t, realInfo.Name(), testInfo.Name())
				require.Equal(t, realInfo.IsDir(), testInfo.IsDir())
				// The size of a directory is system-dependent, so only compare sizes for files.
				if !realInfo.IsDir() {
					require.Equal(t, realInfo.Size(), testInfo.Size())
				}
			}
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
			expectedMode:  0777,
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
