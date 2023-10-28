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
	"os"
	"os/exec"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"go.chromium.org/luci/auth/client/authcli"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		nodePath     string
		npmPath      string
		ctsDir       string
		cacheListOut string
		authFlags    authcli.Flags
	}
}

func (cmd) Name() string { return "build-cache" }

func (cmd) Desc() string { return "builds the CTS test case cache and uploads it to GCP" }

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	dawnRoot := fileutils.DawnRoot()
	ctsPath := filepath.Join(dawnRoot, "third_party", "webgpu-cts")
	cacheListOut := filepath.Join(dawnRoot, "third_party", "gn", "webgpu-cts", "cache_list.txt")
	npmPath, _ := exec.LookPath("npm")
	flag.StringVar(&c.flags.nodePath, "node", fileutils.NodePath(), "path to node")
	flag.StringVar(&c.flags.npmPath, "npm", npmPath, "path to npm")
	flag.StringVar(&c.flags.ctsDir, "cts", ctsPath, "path to CTS")
	flag.StringVar(&c.flags.cacheListOut, "out", cacheListOut, "path to cache_list.txt output file")
	c.flags.authFlags.Register(flag.CommandLine, auth.DefaultAuthOptions( /* needsCloudScopes */ true))
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	if err := common.InstallCTSDeps(ctx, c.flags.ctsDir, fileutils.NodePath()); err != nil {
		return err
	}
	list, err := common.BuildCache(ctx, c.flags.ctsDir, c.flags.nodePath, c.flags.npmPath, c.flags.authFlags)
	if err != nil {
		return fmt.Errorf("failed to build cache: %w", err)
	}

	if err := os.WriteFile(c.flags.cacheListOut, []byte(list), 0666); err != nil {
		return fmt.Errorf("failed to write cache to '%v': %w", c.flags.cacheListOut, err)
	}

	return nil
}
