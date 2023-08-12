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

import "flag"

// Config hold the common configuration between sub-commands
type Config struct {
	// Common command line flags
	Flags struct {
		// Emit additional logging
		Verbose bool
		// Don't emit anything, just check that files are up to date
		CheckStale bool
	}
}

func (c *Config) RegisterFlags() {
	flag.BoolVar(&c.Flags.Verbose, "v", false, "print verbose output")
	flag.BoolVar(&c.Flags.CheckStale, "check-stale", false, "don't emit anything, just check that files are up to date")
}
