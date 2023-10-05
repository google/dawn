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
