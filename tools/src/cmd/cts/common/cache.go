// Copyright 2023 The Dawn Authors
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

package common

import (
	"archive/tar"
	"bytes"
	"compress/gzip"
	"context"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"sort"
	"sync"

	"dawn.googlesource.com/dawn/tools/src/glob"
)

// Cache is the result of BuildCache()
type Cache struct {
	// The list of files
	FileList []string
	// The .tar.gz content
	TarGz []byte
}

// BuildCache builds the CTS case cache
func BuildCache(ctx context.Context, ctsDir, nodePath string) (*Cache, error) {
	// Create a temporary directory for cache files
	cacheDir, err := os.MkdirTemp("", "dawn-cts-cache")
	if err != nil {
		return nil, err
	}
	defer os.RemoveAll(cacheDir)

	// Build the case cache .json files with numCPUs concurrent processes
	errs := make(chan error, 8)
	numCPUs := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numCPUs)
	for i := 0; i < numCPUs; i++ {
		go func(i int) {
			defer wg.Done()
			// Run 'src/common/runtime/cmdline.ts' to build the case cache
			cmd := exec.CommandContext(ctx, nodePath,
				"-e", "require('./src/common/tools/setup-ts-in-node.js');require('./src/common/tools/gen_cache.ts');",
				"--", // Start of arguments
				// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
				// and slices away the first two arguments. When running with '-e', args
				// start at 1, so just inject a placeholder argument.
				"placeholder-arg",
				cacheDir,
				"src/webgpu",
				"--verbose",
				"--nth", fmt.Sprintf("%v/%v", i, numCPUs),
			)
			out := &bytes.Buffer{}
			cmd.Stdout = io.MultiWriter(out, os.Stdout)
			cmd.Stderr = out
			cmd.Dir = ctsDir

			if err := cmd.Run(); err != nil {
				errs <- fmt.Errorf("failed to generate case cache: %w\n%v", err, out.String())
			}
		}(i)
	}

	go func() {
		wg.Wait()
		close(errs)
	}()

	for err := range errs {
		return nil, err
	}

	files, err := glob.Glob(filepath.Join(cacheDir, "**.json"))
	if err != nil {
		return nil, fmt.Errorf("failed to glob cached files: %w", err)
	}

	// Absolute path -> relative path
	for i, absPath := range files {
		relPath, err := filepath.Rel(cacheDir, absPath)
		if err != nil {
			return nil, fmt.Errorf("failed to get relative path for '%v': %w", absPath, err)
		}
		files[i] = relPath
	}

	sort.Strings(files)

	tarBuffer := &bytes.Buffer{}
	t := tar.NewWriter(tarBuffer)

	for _, relPath := range files {
		absPath := filepath.Join(cacheDir, relPath)

		fi, err := os.Stat(absPath)
		if err != nil {
			return nil, fmt.Errorf("failed to stat '%v': %w", relPath, err)
		}

		header, err := tar.FileInfoHeader(fi, relPath)
		if err != nil {
			return nil, fmt.Errorf("failed to create tar file info header for '%v': %w", relPath, err)
		}

		header.Name = relPath // Use the relative path

		if err := t.WriteHeader(header); err != nil {
			return nil, fmt.Errorf("failed to write tar header for '%v': %w", relPath, err)
		}

		file, err := os.Open(absPath)
		if err != nil {
			return nil, fmt.Errorf("failed to open  '%v': %w", file, err)
		}
		defer file.Close()

		if _, err := io.Copy(t, file); err != nil {
			return nil, fmt.Errorf("failed to write '%v' to tar: %w", file, err)
		}

		if err := t.Flush(); err != nil {
			return nil, fmt.Errorf("failed to flush tar for '%v': %w", relPath, err)
		}
	}

	if err := t.Close(); err != nil {
		return nil, fmt.Errorf("failed to close the tar: %w", err)
	}

	compressed := &bytes.Buffer{}
	gz, err := gzip.NewWriterLevel(compressed, gzip.BestCompression)
	if err != nil {
		return nil, fmt.Errorf("failed to create a gzip writer: %w", err)
	}
	if _, err := gz.Write(tarBuffer.Bytes()); err != nil {
		return nil, fmt.Errorf("failed to write to gzip writer: %w", err)
	}
	if err := gz.Close(); err != nil {
		return nil, fmt.Errorf("failed to close the gzip writer: %w", err)
	}
	return &Cache{files, compressed.Bytes()}, nil
}
