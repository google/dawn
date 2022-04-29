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

// Package result holds types that describe CTS test results.
package result

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
)

// Result holds the result of a CTS test
type Result struct {
	Query    query.Query
	Tags     Tags
	Status   Status
	Duration time.Duration
}

// Format writes the Result to the fmt.State
// The Result is printed as a single line, in the form:
//   <query> <tags> <status>
// This matches the order in which results are sorted.
func (r Result) Format(f fmt.State, verb rune) {
	if len(r.Tags) > 0 {
		fmt.Fprintf(f, "%v %v %v %v", r.Query, TagsToString(r.Tags), r.Status, r.Duration)
	} else {
		fmt.Fprintf(f, "%v %v %v", r.Query, r.Status, r.Duration)
	}
}

// String returns the result as a string
func (r Result) String() string {
	sb := strings.Builder{}
	fmt.Fprint(&sb, r)
	return sb.String()
}

// Compare compares the relative order of r and o, returning:
//  -1 if r should come before o
//   1 if r should come after o
//   0 if r and o are identical
// Note: Result.Duration is not considered in comparison.
func (r Result) Compare(o Result) int {
	a, b := r, o
	switch a.Query.Compare(b.Query) {
	case -1:
		return -1
	case 1:
		return 1
	}
	ta := strings.Join(a.Tags.List(), TagDelimiter)
	tb := strings.Join(b.Tags.List(), TagDelimiter)
	switch {
	case ta < tb:
		return -1
	case ta > tb:
		return 1
	case a.Status < b.Status:
		return -1
	case a.Status > b.Status:
		return 1
	}
	return 0
}

// Parse parses the result from a string of the form:
//    <query> <tags> <status>
// <tags> may be omitted if there were no tags.
func Parse(in string) (Result, error) {
	line := in
	token := func() string {
		for i, c := range line {
			if c != ' ' {
				line = line[i:]
				break
			}
		}
		for i, c := range line {
			if c == ' ' {
				tok := line[:i]
				line = line[i:]
				return tok
			}
		}
		tok := line
		line = ""
		return tok
	}

	a := token()
	b := token()
	c := token()
	d := token()
	if a == "" || b == "" || c == "" || token() != "" {
		return Result{}, fmt.Errorf("unable to parse result '%v'", in)
	}

	query := query.Parse(a)

	if d == "" {
		status := Status(b)
		duration, err := time.ParseDuration(c)
		if err != nil {
			return Result{}, fmt.Errorf("unable to parse result '%v': %w", in, err)
		}
		return Result{query, nil, status, duration}, nil
	} else {
		tags := StringToTags(b)
		status := Status(c)
		duration, err := time.ParseDuration(d)
		if err != nil {
			return Result{}, fmt.Errorf("unable to parse result '%v': %w", in, err)
		}
		return Result{query, tags, status, duration}, nil
	}
}

// List is a list of results
type List []Result

// Variant is a collection of tags that uniquely identify a test
// configuration (e.g the combination of OS, GPU, validation-modes, etc).
type Variant = Tags

// Variants returns the list of unique tags (variants) across all results.
func (l List) Variants() []Variant {
	tags := container.NewMap[string, Variant]()
	for _, r := range l {
		tags.Add(TagsToString(r.Tags), r.Tags)
	}
	return tags.Values()
}

// TransformTags returns the list of results with the tags transformed using f.
// TransformTags assumes that f will return the same output for the same input.
func (l List) TransformTags(f func(Tags) Tags) List {
	cache := map[string]Tags{}
	out := List{}
	for _, r := range l {
		key := TagsToString(r.Tags)
		tags, cached := cache[key]
		if !cached {
			tags = f(r.Tags.Clone())
			cache[key] = tags
		}
		out = append(out, Result{
			Query:    r.Query,
			Tags:     tags,
			Status:   r.Status,
			Duration: r.Duration,
		})
	}
	return out
}

// ReplaceDuplicates returns a new list with duplicate test results replaced.
// When a duplicate is found, the function f is called with the duplicate
// results. The returned status will be used as the replaced result.
// Merged results will use the average (mean) duration of the duplicates.
func (l List) ReplaceDuplicates(f func(Statuses) Status) List {
	type key struct {
		query query.Query
		tags  string
	}
	// Collect all duplicates
	keyToIndices := map[key][]int{} // key to index
	for i, r := range l {
		k := key{r.Query, TagsToString(r.Tags)}
		keyToIndices[k] = append(keyToIndices[k], i)
	}
	// Resolve duplicates
	type StatusAndDuration struct {
		Status   Status
		Duration time.Duration
	}
	merged := map[key]StatusAndDuration{}
	for key, indices := range keyToIndices {
		statuses := NewStatuses()
		duration := time.Duration(0)
		for _, i := range indices {
			r := l[i]
			statuses.Add(r.Status)
			duration += r.Duration
		}
		status := func() Status {
			if len(statuses) > 1 {
				return f(statuses)
			}
			return statuses.One()
		}()
		duration = duration / time.Duration(len(indices))
		merged[key] = StatusAndDuration{
			Status:   status,
			Duration: duration,
		}
	}
	// Rebuild list
	out := make(List, 0, len(keyToIndices))
	for _, r := range l {
		k := key{r.Query, TagsToString(r.Tags)}
		if sd, ok := merged[k]; ok {
			out = append(out, Result{
				Query:    r.Query,
				Tags:     r.Tags,
				Status:   sd.Status,
				Duration: sd.Duration,
			})
			delete(merged, k) // Remove from map to prevent duplicates
		}
	}
	return out
}

// Sort sorts the list
func (l List) Sort() {
	sort.Slice(l, func(i, j int) bool { return l[i].Compare(l[j]) < 0 })
}

// Filter returns the results that match the given predicate
func (l List) Filter(f func(Result) bool) List {
	out := make(List, 0, len(l))
	for _, r := range l {
		if f(r) {
			out = append(out, r)
		}
	}
	return out
}

// FilterByStatus returns the results that the given status
func (l List) FilterByStatus(status Status) List {
	return l.Filter(func(r Result) bool {
		return r.Status == status
	})
}

// FilterByTags returns the results that have all the given tags
func (l List) FilterByTags(tags Tags) List {
	return l.Filter(func(r Result) bool {
		return r.Tags.ContainsAll(tags)
	})
}

// FilterByVariant returns the results that exactly match the given tags
func (l List) FilterByVariant(tags Tags) List {
	str := TagsToString(tags)
	return l.Filter(func(r Result) bool {
		return len(r.Tags) == len(tags) && TagsToString(r.Tags) == str
	})
}

// Statuses is a set of Status
type Statuses = container.Set[Status]

// NewStatuses returns a new status set with the provided statuses
func NewStatuses(s ...Status) Statuses { return container.NewSet(s...) }

// Statuses returns a set of all the statuses in the list
func (l List) Statuses() Statuses {
	set := NewStatuses()
	for _, r := range l {
		set.Add(r.Status)
	}
	return set
}

// StatusTree is a query tree of statuses
type StatusTree = query.Tree[Status]

// StatusTree returns a query.Tree from the List, with the Status as the tree
// node data.
func (l List) StatusTree() (StatusTree, error) {
	tree := StatusTree{}
	for _, r := range l {
		if err := tree.Add(r.Query, r.Status); err != nil {
			return StatusTree{}, err
		}
	}
	return tree, nil
}

// Load loads the result list from the file with the given path
func Load(path string) (List, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	results, err := Read(file)
	if err != nil {
		return nil, fmt.Errorf("while reading '%v': %w", path, err)
	}
	return results, nil
}

// Save saves the result list to the file with the given path
func Save(path string, results List) error {
	dir := filepath.Dir(path)
	if err := os.MkdirAll(dir, 0777); err != nil {
		return err
	}
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()
	return Write(file, results)
}

// Read reads a result list from the given reader
func Read(r io.Reader) (List, error) {
	scanner := bufio.NewScanner(r)
	l := List{}
	for scanner.Scan() {
		r, err := Parse(scanner.Text())
		if err != nil {
			return nil, err
		}
		l = append(l, r)
	}
	return l, nil
}

// Write writes a result list to the given writer
func Write(w io.Writer, l List) error {
	for _, r := range l {
		if _, err := fmt.Fprintln(w, r); err != nil {
			return err
		}
	}
	return nil
}

// Merge merges and sorts two results lists.
// Duplicates are removed using the Deduplicate() function.
func Merge(a, b List) List {
	merged := make(List, 0, len(a)+len(b))
	merged = append(merged, a...)
	merged = append(merged, b...)
	out := merged.ReplaceDuplicates(Deduplicate)
	out.Sort()
	return out
}

// Deduplicate is the standard algorithm used to de-duplicating mixed results.
// This function is expected to be handed to List.ReplaceDuplicates().
func Deduplicate(s Statuses) Status {
	// If all results have the same status, then use that
	if len(s) == 1 {
		return s.One()
	}

	// Mixed statuses. Replace with something appropriate.
	switch {
	// Crash + * = Crash
	case s.Contains(Crash):
		return Crash
	// Abort + * = Abort
	case s.Contains(Abort):
		return Abort
	// Unknown + * = Unknown
	case s.Contains(Unknown):
		return Unknown
	// RetryOnFailure + ~(Crash | Abort | Unknown) = RetryOnFailure
	case s.Contains(RetryOnFailure):
		return RetryOnFailure
	// Pass + ~(Crash | Abort | Unknown | RetryOnFailure | Slow) = RetryOnFailure
	case s.Contains(Pass):
		return RetryOnFailure
	}
	return Unknown
}
