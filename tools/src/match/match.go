// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package match provides functions for performing filepath [?,*,**] wildcard
// matching.
package match

import (
	"fmt"
	"regexp"
	"strings"
)

// Test is the match predicate returned by New.
type Test func(path string) bool

// New returns a Test function that returns true iff the path matches the
// provided pattern.
//
// pattern uses forward-slashes for directory separators '/', and may use the
// following wildcards:
//
//	?  - matches any single non-separator character
//	*  - matches any sequence of non-separator characters
//	** - matches any sequence of characters including separators
func New(pattern string) (Test, error) {
	// Transform pattern into a regex by replacing the uses of `?`, `*`, `**`
	// with corresponding regex patterns.
	// As the pattern may contain other regex sequences, the string has to be
	// escaped. So:
	// a) Replace the patterns of `?`, `*`, `**` with unique placeholder tokens.
	// b) Escape the expression so that other sequences don't confuse the regex
	//    parser.
	// c) Replace the placeholder tokens with the corresponding regex tokens.

	// Temporary placeholder tokens
	const (
		starstar     = "••"
		star         = "•"
		questionmark = "¿"
	)
	// Check pattern doesn't contain any of our placeholder tokens
	for _, r := range []rune{'•', '¿'} {
		if strings.ContainsRune(pattern, r) {
			return nil, fmt.Errorf("Pattern must not contain '%c'", r)
		}
	}
	// Replace **, * and ? with placeholder tokens
	subbed := pattern
	subbed = strings.ReplaceAll(subbed, "**", starstar)
	subbed = strings.ReplaceAll(subbed, "*", star)
	subbed = strings.ReplaceAll(subbed, "?", questionmark)
	// Escape any remaining regex characters
	escaped := regexp.QuoteMeta(subbed)
	// Insert regex matchers for the substituted tokens
	regex := "^" + escaped + "$"
	regex = strings.ReplaceAll(regex, starstar, ".*")
	regex = strings.ReplaceAll(regex, star, "[^/]*")
	regex = strings.ReplaceAll(regex, questionmark, "[^/]")

	re, err := regexp.Compile(regex)
	if err != nil {
		return nil, fmt.Errorf(`Failed to compile regex "%v" for pattern "%v": %w`, regex, pattern, err)
	}
	return re.MatchString, nil
}
