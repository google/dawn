// Copyright 2023 The Tint Authors.
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

package build

// Config for a single target of a directory
type TargetConfig struct {
	// Override for the output name of this target
	OutputName string
	// Conditionals for this target
	Condition string
	// Additional dependencies to add to this target
	AdditionalDependencies struct {
		// List of internal dependency patterns
		Internal []string
		// List of external dependencies
		External []ExternalDependencyName
	}
}

// Config for a directory
type DirectoryConfig struct {
	// Condition for all targets in the directory
	Condition string
	// Configuration for the 'lib' target
	Lib *TargetConfig
	// Configuration for the 'test' target
	Test *TargetConfig
	// Configuration for the 'test_cmd' target
	TestCmd *TargetConfig `json:"test_cmd"`
	// Configuration for the 'bench' target
	Bench *TargetConfig
	// Configuration for the 'bench_cmd' target
	BenchCmd *TargetConfig `json:"bench_cmd"`
	// Configuration for the 'fuzz' target
	Fuzz *TargetConfig
	// Configuration for the 'fuzz_cmd' target
	FuzzCmd *TargetConfig `json:"fuzz_cmd"`
	// Configuration for the 'cmd' target
	Cmd *TargetConfig
}
