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

package build_cache

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		nodePath     string
		ctsDir       string
		tarGzOut     string
		cacheListOut string
	}
}

func (cmd) Name() string { return "build-cache" }

func (cmd) Desc() string { return "builds the CTS test case cache.tar.gz file" }

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	dawnRoot := fileutils.DawnRoot()
	ctsPath := filepath.Join(dawnRoot, "third_party", "webgpu-cts")
	cacheTarGzPath := filepath.Join(dawnRoot, "webgpu-cts", "cache.tar.gz")
	cacheListPath := filepath.Join(dawnRoot, "third_party", "gn", "webgpu-cts", "cache_list.txt")
	flag.StringVar(&c.flags.nodePath, "node", fileutils.NodePath(), "path to node")
	flag.StringVar(&c.flags.ctsDir, "cts", ctsPath, "path to CTS")
	flag.StringVar(&c.flags.tarGzOut, "out-tar", cacheTarGzPath, "path to cache.tar.gz output file")
	flag.StringVar(&c.flags.cacheListOut, "out-list", cacheListPath, "path to cache_list.txt output file")

	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	cache, err := common.BuildCache(ctx, c.flags.ctsDir, c.flags.nodePath)

	if err != nil {
		return fmt.Errorf("failed to build cache: %w", err)
	}

	if err := ioutil.WriteFile(c.flags.tarGzOut, cache.TarGz, 0666); err != nil {
		return fmt.Errorf("failed to write cache to '%v': %w", c.flags.tarGzOut, err)
	}

	list := strings.Join(cache.FileList, "\n") + "\n"
	if err := ioutil.WriteFile(c.flags.cacheListOut, []byte(list), 0666); err != nil {
		return fmt.Errorf("failed to write cache to '%v': %w", c.flags.cacheListOut, err)
	}

	return nil
}
