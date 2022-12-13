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

// Package expectations provides types and helpers for parsing, updating and
// writing WebGPU expectations files.
//
// See <dawn>/webgpu-cts/expectations.txt for more information.
package expectations

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"sort"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cts/result"
)

// Content holds the full content of an expectations file.
type Content struct {
	Chunks []Chunk
	Tags   Tags
}

// Chunk is an optional comment followed by a run of expectations.
// A chunk ends at the first blank line, or at the transition from an
// expectation to a line-comment.
type Chunk struct {
	Comments     []string     // Line comments at the top of the chunk
	Expectations Expectations // Expectations for the chunk
}

// Tags holds the tag information parsed in the comments between the
// 'BEGIN TAG HEADER' and 'END TAG HEADER' markers.
// Tags are grouped in tag-sets.
type Tags struct {
	// Map of tag-set name to tags
	Sets []TagSet
	// Map of tag name to tag-set and priority
	ByName map[string]TagSetAndPriority
}

// TagSet is a named collection of tags, parsed from the 'TAG HEADER'
type TagSet struct {
	Name string      // Name of the tag-set
	Tags result.Tags // Tags belonging to the tag-set
}

// TagSetAndPriority is used by the Tags.ByName map to identify which tag-set
// a tag belongs to.
type TagSetAndPriority struct {
	// The tag-set that the tag belongs to.
	Set string
	// The declared order of tag in the set.
	// An expectation may only list a single tag from any set. This priority
	// is used to decide which tag(s) should be dropped when multiple tags are
	// found in the same set.
	Priority int
}

// Expectation holds a single expectation line
type Expectation struct {
	Line    int         // The 1-based line number of the expectation
	Bug     string      // The associated bug URL for this expectation
	Tags    result.Tags // Tags used to filter the expectation
	Query   string      // The CTS query
	Status  []string    // The expected result status
	Comment string      // Optional comment at end of line
}

// Expectations are a list of Expectation
type Expectations []Expectation

// Load loads the expectation file at 'path', returning a Content.
func Load(path string) (Content, error) {
	content, err := ioutil.ReadFile(path)
	if err != nil {
		return Content{}, err
	}
	ex, err := Parse(path, string(content))
	if err != nil {
		return Content{}, err
	}
	return ex, nil
}

// Save saves the Content file to 'path'.
func (c Content) Save(path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()

	return c.Write(f)
}

// Clone makes a deep-copy of the Content.
func (c Content) Clone() Content {
	chunks := make([]Chunk, len(c.Chunks))
	for i, c := range c.Chunks {
		chunks[i] = c.Clone()
	}
	return Content{chunks, c.Tags.Clone()}
}

// Empty returns true if the Content has no chunks.
func (c Content) Empty() bool {
	return len(c.Chunks) == 0
}

// EndsInBlankLine returns true if the Content ends with a blank line
func (c Content) EndsInBlankLine() bool {
	return !c.Empty() && c.Chunks[len(c.Chunks)-1].IsBlankLine()
}

// MaybeAddBlankLine appends a new blank line to the content, if the content
// does not already end in a blank line.
func (c *Content) MaybeAddBlankLine() {
	if !c.Empty() && !c.EndsInBlankLine() {
		c.Chunks = append(c.Chunks, Chunk{})
	}
}

// Write writes the Content, in textual form, to the writer w.
func (c Content) Write(w io.Writer) error {
	for _, chunk := range c.Chunks {
		if len(chunk.Comments) == 0 && len(chunk.Expectations) == 0 {
			if _, err := fmt.Fprintln(w); err != nil {
				return err
			}
			continue
		}
		for _, comment := range chunk.Comments {
			if _, err := fmt.Fprintln(w, comment); err != nil {
				return err
			}
		}
		for _, expectation := range chunk.Expectations {
			parts := []string{}
			if expectation.Bug != "" {
				parts = append(parts, expectation.Bug)
			}
			if len(expectation.Tags) > 0 {
				parts = append(parts, fmt.Sprintf("[ %v ]", strings.Join(expectation.Tags.List(), " ")))
			}
			parts = append(parts, expectation.Query)
			parts = append(parts, fmt.Sprintf("[ %v ]", strings.Join(expectation.Status, " ")))
			if expectation.Comment != "" {
				parts = append(parts, expectation.Comment)
			}
			if _, err := fmt.Fprintln(w, strings.Join(parts, " ")); err != nil {
				return err
			}
		}
	}
	return nil
}

// String returns the Content as a string.
func (c Content) String() string {
	sb := strings.Builder{}
	c.Write(&sb)
	return sb.String()
}

// IsCommentOnly returns true if the Chunk contains comments and no expectations.
func (c Chunk) IsCommentOnly() bool {
	return len(c.Comments) > 0 && len(c.Expectations) == 0
}

// IsBlankLine returns true if the Chunk has no comments or expectations.
func (c Chunk) IsBlankLine() bool {
	return len(c.Comments) == 0 && len(c.Expectations) == 0
}

// Clone returns a deep-copy of the Chunk
func (c Chunk) Clone() Chunk {
	comments := make([]string, len(c.Comments))
	for i, c := range c.Comments {
		comments[i] = c
	}
	expectations := make([]Expectation, len(c.Expectations))
	for i, e := range c.Expectations {
		expectations[i] = e.Clone()
	}
	return Chunk{comments, expectations}
}

// Clone returns a deep-copy of the Tags
func (t Tags) Clone() Tags {
	out := Tags{}
	if t.ByName != nil {
		out.ByName = make(map[string]TagSetAndPriority, len(t.ByName))
		for n, t := range t.ByName {
			out.ByName[n] = t
		}
	}
	if t.Sets != nil {
		out.Sets = make([]TagSet, len(t.Sets))
		copy(out.Sets, t.Sets)
	}
	return out
}

// Clone makes a deep-copy of the Expectation.
func (e Expectation) Clone() Expectation {
	out := Expectation{
		Line:    e.Line,
		Bug:     e.Bug,
		Query:   e.Query,
		Comment: e.Comment,
	}
	if e.Tags != nil {
		out.Tags = e.Tags.Clone()
	}
	if e.Status != nil {
		out.Status = append([]string{}, e.Status...)
	}
	return out
}

// Compare compares the relative order of a and b, returning:
//
//	-1 if a should come before b
//	 1 if a should come after b
//	 0 if a and b are identical
//
// Note: Only comparing bug, query, and tags (in that order).
func (a Expectation) Compare(b Expectation) int {
	switch strings.Compare(a.Bug, b.Bug) {
	case -1:
		return -1
	case 1:
		return 1
	}
	switch strings.Compare(a.Query, b.Query) {
	case -1:
		return -1
	case 1:
		return 1
	}
	aTag := result.TagsToString(a.Tags)
	bTag := result.TagsToString(b.Tags)
	switch strings.Compare(aTag, bTag) {
	case -1:
		return -1
	case 1:
		return 1
	}
	return 0
}

func (l Expectations) Sort() {
	sort.Slice(l, func(i, j int) bool { return l[i].Compare(l[j]) < 0 })
}
