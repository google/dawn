// Copyright 2023 The Dawn & Tint Authors
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
	"strings"
	"sync"

	"cloud.google.com/go/storage"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"go.chromium.org/luci/auth"
	"go.chromium.org/luci/auth/client/authcli"
	"go.chromium.org/luci/common/logging"
	"golang.org/x/oauth2"
	"google.golang.org/api/option"
)

const gcpBucket = "dawn-webgpu-cts-cache"

// BuildCache builds the CTS case cache and uploads it to gcpBucket.
// Returns the cache file list
// Note: InstallCTSDeps() should be called before this function to ensure all
// CTS dependencies are installed
func BuildCache(ctx context.Context, ctsDir, nodePath, npmPath string, authFlags authcli.Flags) (string, error) {
	ctsHash, err := gitHashOf(ctsDir)
	if err != nil {
		return "", fmt.Errorf("failed to get CTS hash: %w", err)
	}

	ts, err := tokenSource(ctx, authFlags)
	if err != nil {
		return "", err
	}

	client, err := storage.NewClient(ctx, option.WithTokenSource(ts),
		option.WithScopes(storage.ScopeReadWrite))
	if err != nil {
		return "", fmt.Errorf("failed to create google cloud storage client: %w", err)
	}
	bucket := client.Bucket(gcpBucket)

	// Create a temporary directory for cache files
	cacheDir, err := os.MkdirTemp("", "dawn-cts-cache")
	if err != nil {
		return "", err
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
			args := []string{
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
			}
			cmd := exec.CommandContext(ctx, nodePath, args...)
			out := &bytes.Buffer{}
			cmd.Stdout = io.MultiWriter(out, os.Stdout)
			cmd.Stderr = out
			cmd.Dir = ctsDir

			if err := cmd.Run(); err != nil {
				errs <- fmt.Errorf("failed to generate case cache:\n\n%v %v\n\n%w\n\n%v",
					nodePath, strings.Join(args, " "), err, out.String())
			}
		}(i)
	}

	go func() {
		wg.Wait()
		close(errs)
	}()

	for err := range errs {
		return "", err
	}

	files, err := glob.Glob(filepath.Join(cacheDir, "**.bin"))
	if err != nil {
		return "", fmt.Errorf("failed to glob cached files: %w", err)
	}

	// Absolute path -> relative path
	for i, absPath := range files {
		relPath, err := filepath.Rel(cacheDir, absPath)
		if err != nil {
			return "", fmt.Errorf("failed to get relative path for '%v': %w", absPath, err)
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
			return "", fmt.Errorf("failed to stat '%v': %w", relPath, err)
		}

		header, err := tar.FileInfoHeader(fi, relPath)
		if err != nil {
			return "", fmt.Errorf("failed to create tar file info header for '%v': %w", relPath, err)
		}

		header.Name = relPath // Use the relative path

		if err := t.WriteHeader(header); err != nil {
			return "", fmt.Errorf("failed to write tar header for '%v': %w", relPath, err)
		}

		file, err := os.Open(absPath)
		if err != nil {
			return "", fmt.Errorf("failed to open  '%v': %w", file, err)
		}
		defer file.Close()

		if _, err := io.Copy(t, file); err != nil {
			return "", fmt.Errorf("failed to write '%v' to tar: %w", file, err)
		}

		if err := t.Flush(); err != nil {
			return "", fmt.Errorf("failed to flush tar for '%v': %w", relPath, err)
		}
	}

	if err := t.Close(); err != nil {
		return "", fmt.Errorf("failed to close the tar: %w", err)
	}

	compressed := &bytes.Buffer{}
	gz, err := gzip.NewWriterLevel(compressed, gzip.BestCompression)
	if err != nil {
		return "", fmt.Errorf("failed to create a gzip writer: %w", err)
	}
	if _, err := gz.Write(tarBuffer.Bytes()); err != nil {
		return "", fmt.Errorf("failed to write to gzip writer: %w", err)
	}
	if err := gz.Close(); err != nil {
		return "", fmt.Errorf("failed to close the gzip writer: %w", err)
	}

	// Write the case list
	list := bucket.Object(ctsHash + "/list")
	listWriter := list.NewWriter(ctx)

	fileList := strings.Join(files, "\n") + "\n"
	if _, err := listWriter.Write([]byte(fileList)); err != nil {
		return "", fmt.Errorf("failed to write to the bucket object: %w", err)
	}
	if err := listWriter.Close(); err != nil {
		return "", fmt.Errorf("failed to close the bucket object: %w", err)
	}

	// Write the data
	data := bucket.Object(ctsHash + "/data")
	dataWriter := data.NewWriter(ctx)
	if _, err := dataWriter.Write(compressed.Bytes()); err != nil {
		return "", fmt.Errorf("failed to write to the bucket object: %w", err)
	}
	if err := dataWriter.Close(); err != nil {
		return "", fmt.Errorf("failed to close the bucket object: %w", err)
	}

	return fileList, nil
}

// tokenSource returns a source of OAuth2 tokens (based on CLI flags) or auth.ErrLoginRequired if the user needs to login first.
func tokenSource(ctx context.Context, flags authcli.Flags) (oauth2.TokenSource, error) {
	opts, err := flags.Options()
	if err != nil {
		return nil, fmt.Errorf("")
	}
	authn := auth.NewAuthenticator(ctx, auth.SilentLogin, opts)
	if email, err := authn.GetEmail(); err == nil {
		logging.Infof(ctx, "Running as %s", email)
	}
	ts, err := authn.TokenSource()
	if err == auth.ErrLoginRequired {
		if err := authn.Login(); err != nil {
			return nil, err
		}
	}
	ts, err = authn.TokenSource()
	if err != nil {
		return nil, err
	}
	return ts, nil
}

func gitHashOf(dir string) (string, error) {
	g, err := git.New("")
	if err != nil {
		return "", fmt.Errorf("failed to create a git api: %w", err)
	}
	r, err := g.Open(dir)
	if err != nil {
		return "", fmt.Errorf("failed to open git repo: %w", err)
	}
	log, err := r.Log(&git.LogOptions{From: "HEAD", To: "HEAD"})
	if err != nil {
		return "", fmt.Errorf("failed to obtain git log: %w", err)
	}
	return log[0].Hash.String(), nil
}
