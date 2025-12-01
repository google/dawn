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
	"io"
	"io/fs"
	"os"
	"path"
	"path/filepath"
	"strings"
	"sync/atomic"
	"syscall"
	"testing/fstest"
	"time"
)

var (
	ErrPwdNotSet  = errors.New("PWD not set in test environment")
	ErrHomeNotSet = errors.New("HOME not set in test environment")
)

// --- OSWrapper implementation ---

// FSTestOSWrapper is an in-memory implementation of OSWrapper for testing.
// It uses a map to simulate a filesystem, which can be used to create
// a read-only fstest.MapFS for read operations. Write operations
// manipulate the map directly.
// Caveats/known differences from real os.* functions:
//   - Paths in error messages will never have a leading / even if it was in
//     the provided path. For example, ReadFile("/root") when /root does not
//     exist will have the error message "open root: file does not exist". This
//     is a side-effect of fs.FS requiring that Open() reject names that do not
//     satisfy fs.ValidPath(), which requires that there is no leading /.
//     TODO(crbug.com/436025865): Update the implementation to correctly add
//     leading / back in.
//   - The current working directory is not taken into account. For example,
//     creating /root/subdir/file.txt, changing to /root/subdir, and attempting
//     to open file.txt will fail. This cannot currently be easily worked
//     around at the moment since the CWD-aware logic lives in EnvironProvider
//     while filesystem operations live in FilesystemReaderWriter.
//     TODO(crbug.com/436025865): Merge everything into FSTestOSWrapper so that
//     the filesystem can be CWD-aware.
type FSTestOSWrapper struct {
	FSTestEnvironProvider
	FSTestFilesystemReaderWriter
}

// CreateFSTestOSWrapper creates a new FSTestOSWrapper with an empty in-memory filesystem.
func CreateFSTestOSWrapper() FSTestOSWrapper {
	return FSTestOSWrapper{
		FSTestEnvironProvider{env: make(map[string]string)},
		FSTestFilesystemReaderWriter{
			FS: make(map[string]*fstest.MapFile),
		},
	}
}

// CreateFSTestOSWrapperWithRealEnv creates a new FSTestOSWrapper with an empty in-memory filesystem
// and an environment initialized from the current process's environment.
func CreateFSTestOSWrapperWithRealEnv() FSTestOSWrapper {
	envMap := make(map[string]string)
	for _, e := range os.Environ() {
		pair := strings.SplitN(e, "=", 2)
		if len(pair) == 2 {
			envMap[pair[0]] = pair[1]
		}
	}

	// os.Getwd() and os.UserHomeDir() are not always from environment variables on all systems, but
	// to simplify the test wrapper PWD and HOME will be populated from them.
	if _, ok := envMap["PWD"]; !ok {
		if wd, err := os.Getwd(); err == nil {
			envMap["PWD"] = wd
		}
	}
	if _, ok := envMap["HOME"]; !ok {
		if home, err := os.UserHomeDir(); err == nil {
			envMap["HOME"] = home
		}
	}

	return FSTestOSWrapper{
		FSTestEnvironProvider: FSTestEnvironProvider{env: envMap},
		FSTestFilesystemReaderWriter: FSTestFilesystemReaderWriter{
			FS: make(map[string]*fstest.MapFile),
		},
	}
}

// --- EnvironProvider implementation ---

// FSTestEnvironProvider is a stub implementation of EnvironProvider that uses a map.
type FSTestEnvironProvider struct {
	env map[string]string
}

// EnvMap returns the internal environment map.
// This is intended for testing purposes.
func (p FSTestEnvironProvider) EnvMap() map[string]string {
	return p.env
}

// Setenv sets an environment variable in the test provider.
func (p FSTestEnvironProvider) Setenv(key, value string) {
	p.env[key] = value
}

func (p FSTestEnvironProvider) Environ() []string {
	result := make([]string, 0, len(p.env))
	for k, v := range p.env {
		result = append(result, k+"="+v)
	}
	return result
}

func (p FSTestEnvironProvider) Getenv(key string) string {
	// os.Getenv returns "" for a missing key, which is the same as a map lookup.
	return p.env[key]
}

func (p FSTestEnvironProvider) Getwd() (string, error) {
	if wd, ok := p.env["PWD"]; ok {
		return wd, nil
	}
	// The specific error returned by os.Getwd is very system dependent, so just returning a package
	// error
	return "", ErrPwdNotSet
}

func (p FSTestEnvironProvider) UserHomeDir() (string, error) {
	if home, ok := p.env["HOME"]; ok {
		return home, nil
	}
	// The specific error returned by os.UserHomeDir() is very system dependent, so just returning
	// a package error
	return "", ErrHomeNotSet
}

// --- FilesystemReaderWriter implementation ---

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

// mapFileInfo wraps a fstest.MapFile to implement the os.FileInfo interface.
// It holds a pointer to the MapFile and the file's full path to derive its base name.
type mapFileInfo struct {
	path string
	file *fstest.MapFile
}

// Name returns the base name of the file.
func (i *mapFileInfo) Name() string {
	return filepath.Base(i.path)
}

// Size returns the length of the file's data in bytes.
func (i *mapFileInfo) Size() int64 {
	return int64(len(i.file.Data))
}

// Mode returns the file mode bits.
func (i *mapFileInfo) Mode() fs.FileMode {
	return i.file.Mode
}

// ModTime returns the modification time.
func (i *mapFileInfo) ModTime() time.Time {
	return i.file.ModTime
}

// IsDir returns true if the entry is a directory.
func (i *mapFileInfo) IsDir() bool {
	return i.file.Mode.IsDir()
}

// Sys returns the underlying data source (can be nil).
func (i *mapFileInfo) Sys() any {
	return i.file.Sys
}

// fstestFileHandle implements the oswrapper.File interface for the in-memory fstest map.
type fstestFileHandle struct {
	path         string
	originalPath string
	info         os.FileInfo
	fsMap        *map[string]*fstest.MapFile
	content      []byte
	offset       int64
	flag         int
	closed       bool
}

// --- FilesystemReader implementation ---

func (w FSTestFilesystemReaderWriter) Open(name string) (File, error) {
	return w.OpenFile(name, os.O_RDONLY, 0)
}

func (w FSTestFilesystemReaderWriter) OpenFile(name string, flag int, perm os.FileMode) (File, error) {
	path := w.CleanPath(name)
	mapFile, exists := w.FS[path]

	// Check if a parent component of the path is a file.
	dir := filepath.Dir(path)
	if dir != "." && dir != "" {
		parentInfo, parentExists := w.FS[dir]
		if parentExists && !parentInfo.Mode.IsDir() {
			return nil, &os.PathError{Op: "open", Path: name, Err: fmt.Errorf("not a directory")}
		}
	}

	if flag&os.O_CREATE != 0 {
		if exists {
			if flag&os.O_EXCL != 0 {
				return nil, &os.PathError{Op: "open", Path: name, Err: os.ErrExist}
			}
		} else {
			parent := filepath.Dir(path)
			if parent != "." && parent != "" {
				parentInfo, parentExists := w.FS[parent]
				if !parentExists {
					return nil, &os.PathError{Op: "open", Path: name, Err: os.ErrNotExist}
				}
				if !parentInfo.Mode.IsDir() {
					return nil, &os.PathError{Op: "open", Path: name, Err: fmt.Errorf("not a directory")}
				}
			}
			newFile := &fstest.MapFile{Data: []byte{}, Mode: perm, ModTime: time.Now()}
			w.FS[path] = newFile
			mapFile = newFile
			exists = true
		}
	}

	if !exists {
		return nil, &os.PathError{Op: "open", Path: name, Err: os.ErrNotExist}
	}

	if mapFile.Mode.IsDir() {
		if flag&(os.O_WRONLY|os.O_RDWR) != 0 {
			return nil, &os.PathError{Op: "open", Path: name, Err: fmt.Errorf("is a directory")}
		}
		if flag&os.O_TRUNC != 0 {
			return nil, &os.PathError{Op: "open", Path: name, Err: fmt.Errorf("is a directory")}
		}
	}

	// Create an os.FileInfo compatible wrapper for the fstest.MapFile.
	info := &mapFileInfo{path: name, file: mapFile}
	handle := &fstestFileHandle{
		path:         path,
		originalPath: name,
		info:         info,
		fsMap:        &w.FS,
		flag:         flag,
		content:      nil, // Set below
		offset:       0,   // Set below
	}

	initialData := mapFile.Data
	if flag&os.O_TRUNC != 0 {
		initialData = []byte{}
	}

	handle.content = make([]byte, len(initialData))
	copy(handle.content, initialData)

	if flag&os.O_APPEND != 0 {
		handle.offset = int64(len(handle.content))
	}

	return handle, nil
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
	fsRoot := w.CleanPath(root)

	walkDirFn := func(path string, d fs.DirEntry, err error) error {
		// The path from fs.WalkDir is relative to the FS root.
		// We need to reconstruct the path that the user's filepath.WalkFunc expects.
		// The contract for filepath.Walk is that the paths passed to the callback
		// have the original `root` argument as a prefix.
		var fullPath string
		if path == fsRoot {
			fullPath = root
		} else {
			// fs.WalkDir gives a path from the FS root. We need the part relative
			// to the walk's root, and then join it with the original root string.
			rel, errRel := filepath.Rel(fsRoot, path)
			if errRel != nil {
				return fn(path, nil, fmt.Errorf("internal error creating relative path: %w", errRel))
			}
			fullPath = filepath.Join(root, rel)
		}

		if err != nil {
			return fn(fullPath, nil, err)
		}
		info, errInfo := d.Info()
		if errInfo != nil {
			return fn(fullPath, nil, errInfo)
		}
		return fn(fullPath, info, nil)
	}
	return fs.WalkDir(w.fs(), fsRoot, walkDirFn)
}

// --- FilesystemWriter implementation ---

func (w FSTestFilesystemReaderWriter) Create(name string) (File, error) {
	return w.OpenFile(name, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
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

// tempCounter is an incrementing value used for generating the 'random' part of  temporary file
// names.
var tempCounter atomic.Uint32

func (w FSTestFilesystemReaderWriter) MkdirTemp(dir, pattern string) (string, error) {
	prefix, suffix := pattern, ""
	if i := strings.LastIndex(pattern, "*"); i != -1 {
		prefix, suffix = pattern[:i], pattern[i+1:]
	}

	// Note: This is using deterministic values for the 'random' part of the path names. This code
	// should only be used for testing purposes, since an attacker might be able to exploit this.
	rndStr := fmt.Sprintf("%d", tempCounter.Add(1))

	name := prefix + rndStr + suffix
	path := filepath.Join(dir, name)
	err := w.Mkdir(path, 0700|os.ModeDir)
	if err != nil {
		return "", err
	}
	return path, nil
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

func (w FSTestFilesystemReaderWriter) Symlink(oldname, newname string) error {
	p := w.CleanPath(newname)

	if _, exists := w.FS[p]; exists {
		return &os.PathError{Op: "symlink", Path: newname, Err: os.ErrExist}
	}

	parent := filepath.Dir(p)
	if parent != "." && parent != "" {
		parentInfo, parentExists := w.FS[parent]
		if !parentExists {
			return &os.PathError{Op: "symlink", Path: newname, Err: os.ErrNotExist}
		}
		if !parentInfo.Mode.IsDir() {
			return &os.PathError{Op: "symlink", Path: newname, Err: fmt.Errorf("not a directory")}
		}
	}

	// For symlinks, the Data field holds the target path.
	// We do NOT clean oldname (the target), as it might be a relative path intended
	// to be resolved relative to the link's location, or an absolute path.
	// We store it exactly as provided.
	w.FS[p] = &fstest.MapFile{
		Data:    []byte(oldname),
		Mode:    fs.ModeSymlink | 0777,
		ModTime: time.Now(),
	}
	return nil
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

// --- File implementation ---

func (h *fstestFileHandle) Stat() (os.FileInfo, error) {
	if h.closed {
		return nil, &os.PathError{Op: "stat", Path: h.originalPath, Err: os.ErrClosed}
	}
	return h.info, nil
}

func (h *fstestFileHandle) Close() error {
	if h.closed {
		return &os.PathError{Op: "close", Path: h.originalPath, Err: os.ErrClosed}
	}
	h.closed = true

	// Only flush content back to the map if the file was opened for writing.
	if h.flag&(os.O_WRONLY|os.O_RDWR) != 0 {
		if mapFile, exists := (*h.fsMap)[h.path]; exists {
			mapFile.Data = h.content
			mapFile.ModTime = time.Now()
		}
	}
	return nil
}

func (h *fstestFileHandle) Read(p []byte) (int, error) {
	if h.closed {
		return 0, &os.PathError{Op: "read", Path: h.originalPath, Err: os.ErrClosed}
	}
	if h.flag&os.O_WRONLY != 0 {
		return 0, &os.PathError{Op: "read", Path: h.originalPath, Err: os.ErrInvalid}
	}

	if h.offset >= int64(len(h.content)) {
		return 0, io.EOF
	}

	n := copy(p, h.content[h.offset:])
	h.offset += int64(n)
	return n, nil
}

func (h *fstestFileHandle) Write(p []byte) (int, error) {
	if h.closed {
		return 0, &os.PathError{Op: "write", Path: h.originalPath, Err: os.ErrClosed}
	}
	if h.flag&(os.O_WRONLY|os.O_RDWR) == 0 {
		return 0, &os.PathError{Op: "write", Path: h.originalPath, Err: os.ErrInvalid}
	}

	if h.flag&os.O_APPEND != 0 {
		h.offset = int64(len(h.content))
	}

	n := len(p)
	end := h.offset + int64(n)
	if end > int64(len(h.content)) {
		newContent := make([]byte, end)
		copy(newContent, h.content)
		h.content = newContent
	}

	copy(h.content[h.offset:end], p)
	h.offset = end
	return n, nil
}

func (h *fstestFileHandle) Seek(offset int64, whence int) (int64, error) {
	if h.closed {
		return 0, &os.PathError{Op: "seek", Path: h.originalPath, Err: os.ErrClosed}
	}
	var newOffset int64
	switch whence {
	case io.SeekStart:
		newOffset = offset
	case io.SeekCurrent:
		newOffset = h.offset + offset
	case io.SeekEnd:
		newOffset = int64(len(h.content)) + offset
	default:
		return 0, &os.PathError{Op: "seek", Path: h.originalPath, Err: os.ErrInvalid}
	}

	if newOffset < 0 {
		return 0, &os.PathError{Op: "seek", Path: h.originalPath, Err: os.ErrInvalid}
	}
	h.offset = newOffset
	return h.offset, nil
}
