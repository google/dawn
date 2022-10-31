// Copyright 2022 The Dawn Authors
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

// cts is a collection of sub-commands for operating on the WebGPU CTS.
//
// To view available commands run: '<dawn>/tools/run cts --help'
package main

import (
	"context"
	"fmt"
	"os"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/subcmd"

	// Register sub-commands
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/export"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/format"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/merge"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/results"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/roll"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/time"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/update"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/validate"
)

func main() {
	ctx := context.Background()

	cfg, err := common.LoadConfig(filepath.Join(fileutils.ThisDir(), "config.json"))
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	if err := subcmd.Run(ctx, *cfg, common.Commands()...); err != nil {
		if err != subcmd.ErrInvalidCLA {
			fmt.Fprintln(os.Stderr, err)
		}
		os.Exit(1)
	}
}
