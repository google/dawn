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

package result

import (
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
)

// Tags is a collection of strings used to annotate test results with the test
// configuration.
type Tags = container.Set[string]

// Returns a new tag set with the given tags
func NewTags(tags ...string) Tags {
	return Tags(container.NewSet(tags...))
}

// TagsToString returns the tags sorted and joined using the TagDelimiter
func TagsToString(t Tags) string {
	return strings.Join(t.List(), TagDelimiter)
}

// StringToTags returns the tags sorted and joined using the TagDelimiter
func StringToTags(s string) Tags {
	return NewTags(strings.Split(s, TagDelimiter)...)
}

// The delimiter used to separate tags when stored as a string
const TagDelimiter = ","
