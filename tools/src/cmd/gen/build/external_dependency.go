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

import "dawn.googlesource.com/dawn/tools/src/match"

// ExternalDependency describes a dependency on an external library
type ExternalDependency struct {
	// Name of the library.
	// See 'externals.json'
	Name ExternalDependencyName
	// The optional condition for using this dependency
	Condition string
	// Include file pattern matcher
	includePatternMatch match.Test
}

// Name of an external dependency
// See 'externals.json'
type ExternalDependencyName string
