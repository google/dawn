// Copyright 2021 The Tint Authors.
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

// gen generates code for the Tint project.
package main

import (
	"context"
	"fmt"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/build"
	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/cmd/gen/templates"
	"dawn.googlesource.com/dawn/tools/src/subcmd"

	// Register sub-commands
	_ "dawn.googlesource.com/dawn/tools/src/cmd/gen/templates"
)

func main() {
	ctx := context.Background()

	if len(os.Args) == 1 || strings.HasPrefix(os.Args[1], "-") {
		os.Args = append([]string{os.Args[0], "all"}, os.Args[1:]...)
	}

	cfg := &common.Config{}
	cfg.RegisterFlags()

	if err := subcmd.Run(ctx, cfg, common.Commands()...); err != nil {
		if err != subcmd.ErrInvalidCLA {
			fmt.Fprintln(os.Stderr, err)
		}
		os.Exit(1)
	}
}

func init() {
	common.Register(&cmdAll{})
}

type cmdAll struct {
}

func (cmdAll) Name() string {
	return "all"
}

func (cmdAll) Desc() string {
	return `all runs all the generators`
}

func (c *cmdAll) RegisterFlags(ctx context.Context, cfg *common.Config) ([]string, error) {
	return nil, nil
}

func (c cmdAll) Run(ctx context.Context, cfg *common.Config) error {
	templatesCmd := templates.Cmd{}
	if err := templatesCmd.Run(ctx, cfg); err != nil {
		return err
	}
	buildCmd := build.Cmd{}
	if err := buildCmd.Run(ctx, cfg); err != nil {
		return err
	}
	return nil
}
