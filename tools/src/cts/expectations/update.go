// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package expectations

import (
	"errors"
	"fmt"
	"log"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"github.com/mattn/go-isatty"
)

// Update performs an incremental update on the expectations using the provided
// results.
//
// Update will:
//   - Remove any expectation lines that have a query where no results match.
//   - Remove expectations lines that are in a chunk which is not annotated with
//     'KEEP', and all test results have the status 'Pass'.
//   - Remove chunks that have had all expectation lines removed.
//   - Appends new chunks for flaky and failing tests which are not covered by
//     existing expectation lines.
//
// Update returns a list of diagnostics for things that should be addressed.
//
// Note: Validate() should be called before attempting to update the
// expectations. If Validate() returns errors, then Update() behaviour is
// undefined.
func (c *Content) Update(results result.List, testlist []query.Query) (Diagnostics, error) {
	// Make a copy of the results. This code mutates the list.
	results = append(result.List{}, results...)

	// Replace statuses that the CTS runner doesn't recognize with 'Failure'
	simplifyStatuses(results)

	// Produce a list of tag sets.
	// We reverse the declared order, as webgpu-cts/expectations.txt lists the
	// most important first (OS, GPU, etc), and result.MinimalVariantTags will
	// prioritize folding away the earlier tag-sets.
	tagSets := make([]result.Tags, len(c.Tags.Sets))
	for i, s := range c.Tags.Sets {
		tagSets[len(tagSets)-i-1] = s.Tags
	}

	// Scan the full result list to obtain all the test variants
	// (unique tag combinations).
	variants := results.Variants()

	// Add 'consumed' results for tests that were skipped.
	// This ensures that skipped results are not included in reduced trees.
	results = c.appendConsumedResultsForSkippedTests(results, testlist, variants)

	var pb *progressbar.ProgressBar
	if isatty.IsTerminal(os.Stdout.Fd()) || isatty.IsCygwinTerminal(os.Stderr.Fd()) {
		pb = progressbar.New(os.Stdout, nil)
		defer pb.Stop()
	}

	u := updater{
		in:       *c,
		out:      Content{},
		qt:       newQueryTree(results),
		variants: variants,
		tagSets:  tagSets,
		pb:       pb,
	}

	if err := u.preserveRetryOnFailures(); err != nil {
		return nil, err
	}

	// Update those expectations!
	if err := u.build(); err != nil {
		return nil, fmt.Errorf("while updating expectations: %w", err)
	}

	*c = u.out
	return u.diags, nil
}

// updater holds the state used for updating the expectations
type updater struct {
	in       Content   // the original expectations Content
	out      Content   // newly built expectations Content
	qt       queryTree // the query tree
	variants []container.Set[string]
	diags    []Diagnostic             // diagnostics raised during update
	tagSets  []result.Tags            // reverse-ordered tag-sets of 'in'
	pb       *progressbar.ProgressBar // Progress bar, may be nil
}

// Returns 'results' with additional 'consumed' results for tests that have
// 'Skip' expectations. This fills in gaps for results, preventing tree
// reductions from marking skipped results as failure, which could result in
// expectation collisions.
func (c *Content) appendConsumedResultsForSkippedTests(results result.List,
	testlist []query.Query,
	variants []container.Set[string]) result.List {
	tree := query.Tree[struct{}]{}
	for _, q := range testlist {
		tree.Add(q, struct{}{})
	}
	// For each variant...
	for _, variant := range variants {
		resultsForVariant := container.NewSet[string]()
		for _, result := range results.FilterByVariant(variant) {
			resultsForVariant.Add(result.Query.String())
		}

		// For each expectation...
		for _, c := range c.Chunks {
			for _, ex := range c.Expectations {
				// Does this expectation apply for variant?
				if !variant.ContainsAll(ex.Tags) {
					continue // Nope.
				}

				// Does the expectation contain a Skip status?
				if !container.NewSet(ex.Status...).Contains(string(result.Skip)) {
					continue // Nope.
				}

				// Gather all the tests that apply to the expectation
				glob, _ := tree.Glob(query.Parse(ex.Query))
				for _, qd := range glob {
					// If we don't have a result for the test, then append a
					// synthetic 'consumed' result.
					if !resultsForVariant.Contains(qd.Query.String()) {
						results = append(results, result.Result{
							Query:  qd.Query,
							Tags:   variant,
							Status: consumed,
						})
					}
				}
			}
		}
	}
	return results
}

// simplifyStatuses replaces all result statuses that are not one of
// 'Pass', 'RetryOnFailure', 'Slow', 'Skip' with 'Failure', and also replaces
// 'Skip' results with 'Pass'.
func simplifyStatuses(results result.List) {
	for i, r := range results {
		switch r.Status {
		case result.Pass, result.RetryOnFailure, result.Slow:
			// keep
		case result.Skip:
			// Typically represents a .unimplemented() test
			results[i].Status = result.Pass
		default:
			results[i].Status = result.Failure
		}
	}
}

const (
	// Status used to mark results that have been already handled by an
	// expectation.
	consumed result.Status = "<<consumed>>"
	// Chunk comment for new flakes
	newFlakesComment = "# New flakes. Please triage:"
	// Chunk comment for new failures
	newFailuresComment = "# New failures. Please triage:"
)

// queryTree holds tree of queries to all results (no filtering by tag or
// status). The queryTree is used to glob all the results that match a
// particular query.
type queryTree struct {
	// All the results.
	results result.List
	// consumedAt is a list of line numbers for the i'th result in 'results'
	// Initially all line numbers are 0. When a result is consumed the line
	// number is set.
	consumedAt []int
	// Each tree node holds a list of indices to results.
	tree query.Tree[[]int]
}

// newQueryTree builds the queryTree from the list of results.
func newQueryTree(results result.List) queryTree {
	log.Println("building query tree...")

	// Build a map of query to result indices
	queryToIndices := map[query.Query][]int{}
	for i, r := range results {
		l := queryToIndices[r.Query]
		l = append(l, i)
		queryToIndices[r.Query] = l
	}

	// Construct the query tree to result indices
	tree := query.Tree[[]int]{}
	for query, indices := range queryToIndices {
		if err := tree.Add(query, indices); err != nil {
			// Unreachable: The only error we could get is duplicate data for
			// the same query, which should be impossible.
			panic(err)
		}
	}

	consumedAt := make([]int, len(results))
	return queryTree{results, consumedAt, tree}
}

// glob returns the list of results matching the given tags under (or with) the
// given query.
func (qt *queryTree) glob(q query.Query) (result.List, error) {
	glob, err := qt.tree.Glob(q)
	if err != nil {
		return nil, fmt.Errorf("while gathering results for query '%v': %w", q, err)
	}

	out := result.List{}
	for _, indices := range glob {
		for _, idx := range indices.Data {
			out = append(out, qt.results[idx])
		}
	}

	return out, nil
}

// globTags returns the list of results matching the given tags under (or with)
// the given query.
func (qt *queryTree) globTags(q query.Query, t result.Tags) (result.List, error) {
	glob, err := qt.tree.Glob(q)
	if err != nil {
		return nil, err
	}

	out := result.List{}
	for _, indices := range glob {
		for _, idx := range indices.Data {
			if r := qt.results[idx]; r.Tags.ContainsAll(t) {
				out = append(out, r)
			}
		}
	}
	return out, nil
}

// markAsConsumed marks all the results matching the given tags
// under (or with) the given query, as consumed.
// line is used to record the line at which the results were consumed. If the
// results were consumed as part of generating new expectations then line should
// be 0.
func (qt *queryTree) markAsConsumed(q query.Query, t result.Tags, line int) {
	if glob, err := qt.tree.Glob(q); err == nil {
		for _, indices := range glob {
			for _, idx := range indices.Data {
				r := &qt.results[idx]
				if r.Tags.ContainsAll(t) {
					r.Status = consumed
					qt.consumedAt[idx] = line
				}
			}
		}
	}
}

// preserveRetryOnFailures changes any results matching expectations with a
// RetryOnFailure expectation to RetryOnFailure.
func (u *updater) preserveRetryOnFailures() error {
	// For each expectation...
	for _, c := range u.in.Chunks {
		for _, ex := range c.Expectations {
			// Does the expectation contain a RetryOnFailure status?
			if !container.NewSet(ex.Status...).Contains(string(result.RetryOnFailure)) {
				continue // Nope.
			}

			q := query.Parse(ex.Query)

			glob, err := u.qt.tree.Glob(q)
			if err != nil {
				if errors.As(err, &query.ErrNoDataForQuery{}) {
					// No results for this RetryOnFailure expectation.
					// Flaky tests might have been removed from the CTS.
					// These expectations will be automatically removed by updater.expectation()
					continue
				}
				return err
			}
			for _, indices := range glob {
				for _, idx := range indices.Data {
					if u.qt.results[idx].Tags.ContainsAll(ex.Tags) {
						u.qt.results[idx].Status = result.RetryOnFailure
					}
				}
			}
		}
	}
	return nil
}

type Progress struct {
	totalExpectations  int
	currentExpectation int
}

// build is the updater top-level function.
// build first appends to u.out all chunks from 'u.in' with expectations updated
// using the new results, and then appends any new expectations to u.out.
func (u *updater) build() error {
	progress := Progress{}

	immutableTokens := []string{
		"KEEP",
		"BEGIN TAG HEADER",
		"Last rolled",
	}

	// Bin the chunks into those that contain any of the strings in
	// immutableTokens in the comments and those that do not have these strings.
	immutableChunks, mutableChunks := []Chunk{}, []Chunk{}
	for _, chunk := range u.in.Chunks {
		// Does the chunk comment contain 'KEEP' or 'BEGIN TAG HEADER' ?
		keep := false

	comments:
		for _, l := range chunk.Comments {
			for _, s := range immutableTokens {
				if strings.Contains(l, s) {
					keep = true
					break comments
				}
			}
		}

		if keep {
			immutableChunks = append(immutableChunks, chunk)
		} else {
			mutableChunks = append(mutableChunks, chunk)
		}

		progress.totalExpectations += len(chunk.Expectations)
	}

	log.Println("updating expectation chunks...")

	// Update all the existing chunks in two passes - those that are immutable
	// then those that are mutable. We do this because the former can't be
	// altered and may declare expectations that may collide with later
	// expectations.
	for _, group := range []struct {
		chunks      []Chunk
		isImmutable bool
	}{
		{immutableChunks, true},
		{mutableChunks, false},
	} {
		for _, in := range group.chunks {
			out := u.chunk(in, group.isImmutable, &progress)

			// If all chunk had expectations, but now they've gone, remove the chunk
			if len(in.Expectations) > 0 && len(out.Expectations) == 0 {
				continue
			}

			u.out.Chunks = append(u.out.Chunks, out)
		}
	}

	// Emit new expectations (flaky, failing)
	if err := u.addNewExpectations(); err != nil {
		return fmt.Errorf("failed to add new expectations: %w", err)
	}

	return nil
}

// chunk returns a new Chunk, based on 'in', with the expectations updated.
// isImmutable is true if the chunk is labelled with 'KEEP' and can't be changed.
func (u *updater) chunk(in Chunk, isImmutable bool, progress *Progress) Chunk {
	if len(in.Expectations) == 0 {
		return in // Just a comment / blank line
	}

	// Skip over any untriaged failures / flake chunks.
	// We'll just rebuild them at the end.
	for _, line := range in.Comments {
		if strings.HasPrefix(line, newFailuresComment) ||
			strings.HasPrefix(line, newFlakesComment) {
			return Chunk{}
		}
	}

	// Begin building the output chunk.
	// Copy over the chunk's comments.
	out := Chunk{Comments: in.Comments}

	// Build the new chunk's expectations
	for _, exIn := range in.Expectations {
		if u.pb != nil {
			u.pb.Update(progressbar.Status{Total: progress.totalExpectations, Segments: []progressbar.Segment{
				{Count: 1 + progress.currentExpectation},
			}})
			progress.currentExpectation++
		}

		exOut := u.expectation(exIn, isImmutable)
		out.Expectations = append(out.Expectations, exOut...)
	}

	// Sort the expectations to keep things clean and tidy.
	out.Expectations.Sort()
	return out
}

// expectation returns a new list of Expectations, based on the Expectation 'in',
// using the new result data.
func (u *updater) expectation(in Expectation, immutable bool) []Expectation {
	// noResults is a helper for returning when the expectation has no test
	// results.
	noResults := func() []Expectation {
		if len(in.Tags) > 0 {
			u.diag(Warning, in.Line, "no results found for '%v' with tags %v", in.Query, in.Tags)
		} else {
			u.diag(Warning, in.Line, "no results found for '%v'", in.Query)
		}
		// Remove the no-results expectation
		return []Expectation{}
	}

	q := query.Parse(in.Query)

	// Glob the results for the expectation's query + tag combination.
	// Ensure that none of these are already consumed.
	results, err := u.qt.globTags(q, in.Tags)
	// If we can't find any results for this query + tag combination, then bail.
	switch {
	case errors.As(err, &query.ErrNoDataForQuery{}):
		return noResults()
	case err != nil:
		u.diag(Error, in.Line, "%v", err)
		return []Expectation{}
	case len(results) == 0:
		return noResults()
	}

	// Before returning, mark all the results as consumed.
	// Note: this has to happen *after* we've generated the new expectations, as
	// marking the results as 'consumed' will impact the logic of
	// expectationsForRoot()
	defer u.qt.markAsConsumed(q, in.Tags, in.Line)

	if immutable { // Expectation chunk was marked with 'KEEP'
		// Add a diagnostic if all tests of the expectation were 'Pass'
		if s := results.Statuses(); len(s) == 1 && s.One() == result.Pass {
			if c := len(results); c > 1 {
				u.diag(Note, in.Line, "all %d tests now pass", len(results))
			} else {
				u.diag(Note, in.Line, "test now passes")
			}
		}
		return []Expectation{in}
	}

	// Rebuild the expectations for this query.
	return u.expectationsForRoot(q, in.Line, in.Bug, in.Comment)
}

// addNewExpectations (potentially) appends to 'u.out' chunks for new flaky and
// failing tests.
func (u *updater) addNewExpectations() error {
	// For each variant:
	// • Build a query tree using the results filtered to the variant, and then
	//   reduce the tree.
	// • Take all the reduced-tree leaf nodes, and add these to 'roots'.
	// Once we've collected all the roots, we'll use these to build the
	// expectations across the reduced set of tags.
	log.Println("determining new expectation roots...")
	roots := query.Tree[bool]{}
	for i, variant := range u.variants {
		if u.pb != nil {
			u.pb.Update(progressbar.Status{Total: len(u.variants), Segments: []progressbar.Segment{
				{Count: 1 + i},
			}})
		}

		// Build a tree from the results matching the given variant.
		tree, err := u.qt.results.FilterByVariant(variant).StatusTree()
		if err != nil {
			return fmt.Errorf("while building tree for tags '%v': %w", variant, err)
		}
		// Reduce the tree.
		tree.Reduce(treeReducer)
		// Add all the reduced leaf nodes to 'roots'.
		for _, qd := range tree.List() {
			// Use Split() to ensure that only the leaves have data (true) in the tree
			roots.Split(qd.Query, true)
		}
	}

	// Build all the expectations for each of the roots.
	log.Println("building new expectations...")
	rootsList := roots.List()
	expectations := []Expectation{}
	for i, root := range rootsList {
		if u.pb != nil {
			u.pb.Update(progressbar.Status{Total: len(rootsList), Segments: []progressbar.Segment{
				{Count: 1 + i},
			}})
		}
		expectations = append(expectations, u.expectationsForRoot(
			root.Query,            // Root query
			0,                     // Line number
			"crbug.com/dawn/0000", // Bug
			"",                    // Comment
		)...)
	}

	// Bin the expectations by failure or flake.
	flakes, failures := []Expectation{}, []Expectation{}
	for _, r := range expectations {
		if container.NewSet(r.Status...).Contains(string(result.RetryOnFailure)) {
			flakes = append(flakes, r)
		} else {
			failures = append(failures, r)
		}
	}

	// Create chunks for any flakes and failures, in that order.
	for _, group := range []struct {
		results []Expectation
		comment string
	}{
		{flakes, newFlakesComment},
		{failures, newFailuresComment},
	} {
		if len(group.results) > 0 {
			u.out.Chunks = append(u.out.Chunks, Chunk{
				Comments: []string{
					"################################################################################",
					group.comment,
					"################################################################################",
				},
				Expectations: group.results,
			})
		}
	}

	return nil
}

// expectationsForRoot builds a list of expectations that cover the failing
// tests for the results under root.
// The returned list of expectations is optimized by reducing queries to the
// most common root, and reducing tags to the smallest required set.
func (u *updater) expectationsForRoot(
	root query.Query, // The sub-tree query root
	line int, // The originating line, when producing diagnostics
	bug string, // The bug to apply to all returned expectations
	comment string, // The comment to apply to all returned expectations
) []Expectation {
	results, err := u.qt.glob(root)
	if err != nil {
		u.diag(Error, line, "%v", err)
		return nil
	}

	// Using the full list of unfiltered tests, generate the minimal set of
	// variants (tags) that uniquely classify the results with differing status.
	minimalVariants := u.
		cleanupTags(results).
		MinimalVariantTags(u.tagSets)

	// For each minimized variant...
	reduced := result.List{}
	for _, variant := range minimalVariants {
		// Build a query tree from this variant...
		tree := result.StatusTree{}
		filtered := results.FilterByTags(variant)
		for _, r := range filtered {
			// Note: variants may overlap, but overlaped queries will have
			// identical statuses, so we can just ignore the error for Add().
			tree.Add(r.Query, r.Status)
		}

		// ... and reduce the tree by collapsing sub-trees that have common
		// statuses.
		tree.ReduceUnder(root, treeReducer)

		// Append the reduced tree nodes to the results list
		for _, qs := range tree.List() {
			reduced = append(reduced, result.Result{
				Query:  qs.Query,
				Tags:   variant,
				Status: qs.Data,
			})
		}
	}

	// Filter out any results that passed or have already been consumed
	filtered := reduced.Filter(func(r result.Result) bool {
		return r.Status != result.Pass && r.Status != consumed
	})

	// Mark all the new expectation results as consumed.
	for _, r := range filtered {
		u.qt.markAsConsumed(r.Query, r.Tags, 0)
	}

	// Transform the results to expectations.
	return u.resultsToExpectations(filtered, bug, comment)
}

// resultsToExpectations returns a list of expectations from the given results.
// Each expectation will have the same query, tags and status as the input
// result, along with the specified bug and comment.
//
// If the result query target is a test without a wildcard, then the expectation
// will have a wildcard automatically appended. This is to satisfy a requirement
// of the expectation validator.
func (u *updater) resultsToExpectations(results result.List, bug, comment string) []Expectation {
	results.Sort()

	out := make([]Expectation, len(results))
	for i, r := range results {
		q := r.Query.String()
		if r.Query.Target() == query.Tests && !r.Query.IsWildcard() {
			// The expectation validator wants a trailing ':' for test queries
			q += query.TargetDelimiter
		}
		out[i] = Expectation{
			Bug:     bug,
			Tags:    r.Tags,
			Query:   q,
			Status:  []string{string(r.Status)},
			Comment: comment,
		}
	}

	return out
}

// cleanupTags returns a copy of the provided results with:
//   - All tags not found in the expectations list removed
//   - All but the highest priority tag for any tag-set.
//     The tag sets are defined by the `BEGIN TAG HEADER` / `END TAG HEADER`
//     section at the top of the expectations file.
func (u *updater) cleanupTags(results result.List) result.List {
	return results.TransformTags(func(t result.Tags) result.Tags {
		type HighestPrioritySetTag struct {
			tag      string
			priority int
		}
		// Set name to highest priority tag for that set
		best := map[string]HighestPrioritySetTag{}
		for tag := range t {
			sp, ok := u.in.Tags.ByName[tag]
			if ok {
				if set := best[sp.Set]; sp.Priority >= set.priority {
					best[sp.Set] = HighestPrioritySetTag{tag, sp.Priority}
				}
			}
		}
		t = result.NewTags()
		for _, ts := range best {
			t.Add(ts.tag)
		}
		return t
	})
}

// treeReducer is a function that can be used by StatusTree.Reduce() to reduce
// tree nodes with the same status.
// treeReducer will collapse trees nodes if any of the following are true:
//   - All child nodes have the same status
//   - More than 50% of the child nodes have a non-pass status, and none of the
//     children are consumed.
//   - There are more than 10 child nodes with a non-pass status, and none of the
//     children are consumed.
func treeReducer(statuses []result.Status) *result.Status {
	counts := map[result.Status]int{}
	for _, s := range statuses {
		counts[s] = counts[s] + 1
	}
	if len(counts) == 1 {
		return &statuses[0] // All the same status
	}
	if counts[consumed] > 0 {
		return nil // Partially consumed trees cannot be merged
	}
	highestNonPassCount := 0
	highestNonPassStatus := result.Failure
	for s, n := range counts {
		if s != result.Pass {
			if percent := (100 * n) / len(statuses); percent > 50 {
				// Over 50% of all the children are of non-pass status s.
				return &s
			}
			if n > highestNonPassCount {
				highestNonPassCount = n
				highestNonPassStatus = s
			}
		}
	}

	if highestNonPassCount > 10 {
		// Over 10 child node failed.
		return &highestNonPassStatus
	}

	return nil
}

// diag appends a new diagnostic to u.diags with the given severity, line and
// message.
func (u *updater) diag(severity Severity, line int, msg string, args ...interface{}) {
	u.diags = append(u.diags, Diagnostic{
		Severity: severity,
		Line:     line,
		Message:  fmt.Sprintf(msg, args...),
	})
}
