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

package substr

import (
	diff "github.com/sergi/go-diff/diffmatchpatch"
)

// Fix attempts to reconstruct substr by comparing it to body.
// substr is a fuzzy substring of body.
// Fix returns a new exact substring of body, by calculating a diff of the text.
// If no match could be made, Fix() returns an empty string.
func Fix(body, substr string) string {
	dmp := diff.New()

	diffs := dmp.DiffMain(body, substr, false)
	if len(diffs) == 0 {
		return ""
	}

	front := func() diff.Diff { return diffs[0] }
	back := func() diff.Diff { return diffs[len(diffs)-1] }

	start, end := 0, len(body)

	// Trim edits that remove text from body start
	for len(diffs) > 0 && front().Type == diff.DiffDelete {
		start += len(front().Text)
		diffs = diffs[1:]
	}

	// Trim edits that remove text from body end
	for len(diffs) > 0 && back().Type == diff.DiffDelete {
		end -= len(back().Text)
		diffs = diffs[:len(diffs)-1]
	}

	// New substring is the span for the remainder of the edits
	return body[start:end]
}
