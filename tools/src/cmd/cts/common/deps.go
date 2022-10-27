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

package common

import (
	"fmt"
	"regexp"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
)

// The regular expression used to search for the CTS hash
var reCTSHash = regexp.MustCompile(reEscape(ctsHashPrefix) + `[0-9a-fA-F]+`)

const (
	// The string prefix for the CTS hash in the DEPs file, used for identifying
	// and updating the DEPS file.
	ctsHashPrefix = `{chromium_git}/external/github.com/gpuweb/cts@`
)

func reEscape(s string) string {
	return strings.ReplaceAll(strings.ReplaceAll(s, `/`, `\/`), `.`, `\.`)
}

// UpdateCTSHashInDeps replaces the CTS hashes in 'deps' with 'newCTSHash'.
// Returns:
//
//	newDEPS    - the new DEPS content
//	oldCTSHash - the old CTS hash in the 'deps'
func UpdateCTSHashInDeps(deps, newCTSHash string) (newDEPS, oldCTSHash string, err error) {
	// Collect old CTS hashes, and replace these with newCTSHash
	b := strings.Builder{}
	oldCTSHashes := []string{}
	matches := reCTSHash.FindAllStringIndex(deps, -1)
	if len(matches) == 0 {
		return "", "", fmt.Errorf("failed to find a CTS hash in DEPS file")
	}
	end := 0
	for _, match := range matches {
		oldCTSHashes = append(oldCTSHashes, deps[match[0]+len(ctsHashPrefix):match[1]])
		b.WriteString(deps[end:match[0]])
		b.WriteString(ctsHashPrefix + newCTSHash)
		end = match[1]
	}
	b.WriteString(deps[end:])

	newDEPS = b.String()

	if s := container.NewSet(oldCTSHashes...); len(s) > 1 {
		fmt.Println("DEPS contained multiple hashes for CTS, using first for logs")
	}
	oldCTSHash = oldCTSHashes[0]

	return newDEPS, oldCTSHash, nil
}
