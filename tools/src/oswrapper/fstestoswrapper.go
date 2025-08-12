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

// Contains implementation of oswrapper interfaces using fstest.
package oswrapper

import (
	"errors"
	"fmt"
	"io/fs"
	"os"
	"path"
	"path/filepath"
	"strings"
	"syscall"
	"testing/fstest"
	"time"
)

// FSTestOSWrapper is an in-memory implementation of OSWrapper for testing.
// It uses a map to simulate a filesystem, which can be used to create
// a read-only fstest.MapFS for read operations. Write operations
// manipulate the map directly.
type FSTestOSWrapper struct {
	FSTestEnvironProvider
	FSTestFilesystemReaderWriter
}

// CreateFSTestOSWrapper creates a new FSTestOSWrapper with an empty in-memory filesystem.
func CreateFSTestOSWrapper() FSTestOSWrapper {
	return FSTestOSWrapper{
		FSTestEnvironProvider{},
		FSTestFilesystemReaderWriter{
			FS: make(map[string]*fstest.MapFile),
		},
	}
}

// FSTestEnvironProvider is a stub implementation of EnvironProvider that panics if called.
type FSTestEnvironProvider struct{}

// FSTestFilesystemReaderWriter provides an in-memory implementation of FilesystemReaderWriter.
// It holds the map that represents the filesystem.
type FSTestFilesystemReaderWriter struct {
	FS map[string]*fstest.MapFile
}

// CleanPath converts an OS dependent path into a fs.FS compatible path that can be used as a key
// within FSTestOSWrapper
func (w FSTestFilesystemReaderWriter) CleanPath(pathStr string) string {
	// Replace all backslashes with forward slashes to handle Windows paths on any OS.
	p := strings.ReplaceAll(pathStr, "\\", "/")
	// Use the platform-agnostic path package to clean the path.
	p = path.Clean(p)

	// Normalize the path to be compatible with fs.FS
	p = strings.TrimPrefix(p, "/")
	if p == "" {
		p = "." // The canonical representation of the root in a fs.FS.
	}
	return p
}

// fs returns a read-only fs.FS view of the underlying map.
func (w FSTestFilesystemReaderWriter) fs() fs.FS {
	return fstest.MapFS(w.FS)
}

// --- EnvironProvider implementation ---

func (p FSTestEnvironProvider) Environ() []string {
	panic("Environ() is not currently implemented in fstest wrapper")
}

func (p FSTestEnvironProvider) Getenv(_ string) string {
	panic("Getenv() is not currently implemented in fstest wrapper")
}

func (p FSTestEnvironProvider) Getwd() (string, error) {
	panic("Getwd() is not currently implemented in fstest wrapper")
}

func (p FSTestEnvironProvider) UserHomeDir() (string, error) {
	panic("UserHomeDir() is not currently implemented in fstest wrapper")
}

// --- FilesystemReader implementation ---

func (w FSTestFilesystemReaderWriter) Open(name string) (File, error) {
	panic("Open() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) OpenFile(name string, flag int, perm os.FileMode) (File, error) {
	panic("OpenFile() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) ReadFile(name string) ([]byte, error) {
	p := w.CleanPath(name)

	// If the path is the root, it's always a directory.
	if p == "." {
		return nil, &os.PathError{Op: "read", Path: name, Err: fmt.Errorf("is a directory")}
	}

	// Check for an explicit entry
	if mapFile, exists := w.FS[p]; exists {
		if mapFile.Mode.IsDir() {
			return nil, &os.PathError{Op: "read", Path: name, Err: fmt.Errorf("is a directory")}
		}
		data := make([]byte, len(mapFile.Data))
		copy(data, mapFile.Data)
		return data, nil
	}

	// No check for an implicit entry, since MkdirAll should create all the intermediate
	// directories.

	return nil, &os.PathError{Op: "open", Path: name, Err: os.ErrNotExist}
}

func (w FSTestFilesystemReaderWriter) ReadDir(dir string) ([]os.DirEntry, error) {
	p := w.CleanPath(dir)

	if mapFile, exists := w.FS[p]; exists && !mapFile.Mode.IsDir() {
		return nil, &os.PathError{Op: "readdir", Path: dir, Err: fmt.Errorf("not a directory")}
	}
	return fs.ReadDir(w.fs(), p)
}

func (w FSTestFilesystemReaderWriter) Stat(name string) (os.FileInfo, error) {
	return fs.Stat(w.fs(), w.CleanPath(name))
}

func (w FSTestFilesystemReaderWriter) Walk(root string, fn filepath.WalkFunc) error {
	panic("Walk() is not currently implemented in fstest wrapper")
}

// --- FilesystemWriter implementation ---

func (w FSTestFilesystemReaderWriter) Create(name string) (File, error) {
	panic("Create() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Mkdir(dir string, perm os.FileMode) error {
	p := w.CleanPath(dir)

	if p == "." {
		// The root directory always exists in a fs.FS.
		// os.Mkdir returns EEXIST in this case.
		return &os.PathError{Op: "mkdir", Path: dir, Err: os.ErrExist}
	}

	if _, exists := w.FS[p]; exists {
		return &os.PathError{Op: "mkdir", Path: dir, Err: os.ErrExist}
	}

	parent := filepath.Dir(p)
	if parent != "." {
		parentInfo, parentExists := w.FS[parent]
		if !parentExists {
			return &os.PathError{Op: "mkdir", Path: dir, Err: os.ErrNotExist}
		}
		if !parentInfo.Mode.IsDir() {
			return &os.PathError{Op: "mkdir", Path: dir, Err: fmt.Errorf("not a directory")}
		}
	}
	w.FS[p] = &fstest.MapFile{Mode: os.ModeDir | perm, ModTime: time.Now()}
	return nil
}

func (w FSTestFilesystemReaderWriter) MkdirAll(dir string, perm os.FileMode) error {
	err := w.Mkdir(dir, perm)
	if err == nil {
		return nil
	}

	if os.IsExist(err) {
		info, statErr := w.Stat(dir)
		if statErr == nil && info.IsDir() {
			// p already exists and is a directory, so return success
			return nil
		}
		// p already exists and is not a directory, so propagate the error
		return err
	}

	if !os.IsNotExist(err) {
		// Unexpected failure, probably indicating a bad path, i.e. parent is a file, so propagate the error
		return err
	}

	// At this point, it is known the problem was the parent didn't exist, so need to recursively create it.
	p := w.CleanPath(dir)
	parent := filepath.Dir(p)
	if parent == "." || parent == p {
		// The parent is the root of the filesystem, so Mkdir should have succeeded, so propagating the error.
		return err
	}

	if err := w.MkdirAll(parent, perm); err != nil {
		return err
	}

	// Creating the directory now that the parent exists.
	return w.Mkdir(p, perm)
}

func (w FSTestFilesystemReaderWriter) MkdirTemp(dir, pattern string) (string, error) {
	panic("MkdirTemp() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Remove(name string) error {
	p := w.CleanPath(name)

	info, err := w.Stat(p)
	if err != nil {
		var pathErr *os.PathError
		if errors.As(err, &pathErr) {
			pathErr.Op = "remove"
			pathErr.Path = name
			return pathErr
		}
		return &os.PathError{Op: "remove", Path: name, Err: err}
	}

	if info.IsDir() {
		entries, err := w.ReadDir(p)
		if err != nil {
			return &os.PathError{Op: "remove", Path: name, Err: err}
		}
		if len(entries) > 0 {
			return &os.PathError{Op: "remove", Path: name, Err: syscall.ENOTEMPTY}
		}
	}
	delete(w.FS, p)
	return nil
}

func (w FSTestFilesystemReaderWriter) RemoveAll(path string) error {
	p := w.CleanPath(path)

	// Check if the path or any of its parents are invalid.
	// os.RemoveAll returns nil if the path doesn't exist, but errors if a
	// parent path component is a file.
	dir := filepath.Dir(p)
	for dir != "." && dir != "" {
		info, exists := w.FS[dir]
		if exists && !info.Mode.IsDir() {
			return &os.PathError{Op: "removeall", Path: path, Err: fmt.Errorf("not a directory")}
		}
		dir = filepath.Dir(dir)
	}

	prefix := p + "/"
	for key := range w.FS {
		if key == p || strings.HasPrefix(key, prefix) {
			delete(w.FS, key)
		}
	}
	return nil
}

func (w FSTestFilesystemReaderWriter) Symlink(_, _ string) error {
	panic("Symlink() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) WriteFile(name string, data []byte, perm os.FileMode) error {
	p := w.CleanPath(name)

	if info, exists := w.FS[p]; exists && info.Mode.IsDir() {
		return &os.PathError{Op: "open", Path: name, Err: fmt.Errorf("is a directory")}
	}

	dir := filepath.Dir(p)
	if dir != "." && dir != "" {
		parentInfo, parentExists := w.FS[dir]
		if !parentExists {
			return &os.PathError{Op: "open", Path: name, Err: os.ErrNotExist}
		}
		if !parentInfo.Mode.IsDir() {
			return &os.PathError{Op: "open", Path: name, Err: fmt.Errorf("not a directory")}
		}
	}
	w.FS[p] = &fstest.MapFile{Data: data, Mode: perm, ModTime: time.Now()}
	return nil
}
