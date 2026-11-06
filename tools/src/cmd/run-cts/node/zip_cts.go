// Copyright 2026 The Dawn & Tint Authors
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

package node

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/subcmd"
)

func init() {
	common.Register(&zipCmd{})
}

// zipCmd writes the zip of the built CTS that the standalone runner reads with
// --cts-zip.
type zipCmd struct {
	ctsDir  string // root directory of the CTS
	npx     string // path to the npx executable
	build   bool   // build the CTS first if it is out of date
	zipPath string // path of the zip to write
}

func (c *zipCmd) Name() string {
	return "zip-cts"
}

func (c *zipCmd) Desc() string {
	return "builds the CTS if it is out of date and zips it for " + runnerName + " --cts-zip"
}

func (c *zipCmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	flag.StringVar(&c.ctsDir, "cts", common.DefaultCtsPath(cfg.OsWrapper), "root directory of WebGPU CTS")
	flag.StringVar(&c.npx, "npx", common.DefaultNpxPath(), "path to npx executable")
	flag.BoolVar(&c.build, "build", true, "attempt to build the CTS before zipping it")
	flag.StringVar(&c.zipPath, "o", "cts.zip", "path of the zip to write")
	return nil, nil
}

func (c *zipCmd) Run(ctx context.Context, cfg common.Config) error {
	if c.ctsDir == "" {
		return subcmd.InvalidCLA()
	}
	if c.build {
		cts := common.NewCTS(c.ctsDir, c.npx, "")
		if err := cts.Node.BuildIfRequired(false, cfg.OsWrapper); err != nil {
			return err
		}
	}
	cmdlineScript := filepath.Join(c.ctsDir, "out-node", "common", "runtime", "cmdline.js")
	if !fileutils.IsFile(cmdlineScript, cfg.OsWrapper) {
		return fmt.Errorf("the CTS in '%v' has not been built: run 'npm ci && npx grunt node' there", c.ctsDir)
	}
	files, err := ctsFiles(c.ctsDir)
	if err != nil {
		return err
	}
	return writeArchive(c.zipPath, c.ctsDir, files, defaultDeflate)
}
