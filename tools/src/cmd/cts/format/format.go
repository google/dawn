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

package format

import (
	"context"
	"flag"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		expectations string // expectations file path
	}
}

func (cmd) Name() string {
	return "format"
}

func (cmd) Desc() string {
	return "formats a WebGPUExpectation file"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	defaultExpectations := common.DefaultExpectationsPath()
	flag.StringVar(&c.flags.expectations, "expectations", defaultExpectations, "path to CTS expectations file to update")
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	ex, err := expectations.Load(c.flags.expectations)
	if err != nil {
		return err
	}
	return ex.Save(c.flags.expectations)
}
