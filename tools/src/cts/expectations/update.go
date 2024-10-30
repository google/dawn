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
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"github.com/mattn/go-isatty"
)

// AddExpectationsForFailingResults adds new expectations for the provided
// failing results, with the assumption that the provided results do not
// have existing expectations.
//
// This will:
//   - Remove expectations for non-existent tests.
//   - Reduce result tags down to only the most explicit tag from each set
//   - Merge identical results together
//   - Add new expectations to the one mutable chunk that is expected to
//     be present in the file.
//   - Sort the mutable chunk's expectations by test name, then tags.
//
// TODO(crbug.com/372730248): Return diagnostics.
func (c *Content) AddExpectationsForFailingResults(results result.List,
	testlist []query.Query, verbose bool) error {
	// Make a copy of the results. This code mutates the list.
	results = append(result.List{}, results...)

	startTime := time.Now()
	// TODO(crbug.com/372730248): Do this once (instead of every patchset) and/or
	// find a way to optimize this. This currently takes ~99% of the result
	// processing time.
	if err := c.removeExpectationsForUnknownTests(&testlist); err != nil {
		return err
	}
	if verbose {
		fmt.Printf("Removing unknown expectations took %s\n", time.Now().Sub(startTime).String())
	}

	startTime = time.Now()
	if err := c.removeUnknownTags(&results); err != nil {
		return err
	}
	if verbose {
		fmt.Printf("Removing unknown tags took %s\n", time.Now().Sub(startTime).String())
	}

	startTime = time.Now()
	if err := c.reduceTagsToMostExplicitOnly(&results); err != nil {
		return err
	}
	if verbose {
		fmt.Printf("Reducing tags took %s\n", time.Now().Sub(startTime).String())
	}

	// Merge identical results.
	startTime = time.Now()
	results = result.Merge(results)
	if verbose {
		fmt.Printf("Merging identical results took %s\n", time.Now().Sub(startTime).String())
	}

	startTime = time.Now()
	if err := c.addExpectationsToMutableChunk(&results); err != nil {
		return err
	}
	if verbose {
		fmt.Printf("Adding expectations took %s\n", time.Now().Sub(startTime).String())
	}
	return nil
}

// removeExpectationsForUnknownTests modifies the Content in place so that all
// contained Expectations apply to tests in the given testlist.
func (c *Content) removeExpectationsForUnknownTests(testlist *[]query.Query) error {
	// Converting into a set allows us to much more efficiently check if a
	// non-wildcard expectation is for a valid test.
	knownTestNames := container.NewSet[string]()
	for _, testQuery := range *testlist {
		knownTestNames.Add(testQuery.ExpectationFileString())
	}

	prunedChunkSlice := make([]Chunk, 0)
	for _, chunk := range c.Chunks {
		prunedChunk := chunk.Clone()
		// If we don't have any expectations already, just add the chunk back
		// immediately to avoid removing comments, especially the header.
		if prunedChunk.IsCommentOnly() {
			prunedChunkSlice = append(prunedChunkSlice, prunedChunk)
			continue
		}

		prunedChunk.Expectations = make(Expectations, 0)
		for _, expectation := range chunk.Expectations {
			// We don't actually parse the query string into a Query since wildcards
			// are treated differently between expectations and CTS queries.
			if strings.HasSuffix(expectation.Query, "*") {
				testPrefix := expectation.Query[:len(expectation.Query)-1]
				for testName := range knownTestNames {
					if strings.HasPrefix(testName, testPrefix) {
						prunedChunk.Expectations = append(prunedChunk.Expectations, expectation)
						break
					}
				}
			} else {
				if knownTestNames.Contains(expectation.Query) {
					prunedChunk.Expectations = append(prunedChunk.Expectations, expectation)
				}
			}
		}

		if len(prunedChunk.Expectations) > 0 {
			prunedChunkSlice = append(prunedChunkSlice, prunedChunk)
		}
	}

	c.Chunks = prunedChunkSlice
	return nil
}

// removeUnknownTags modifies |results| in place so that the Results contained
// within it only use tags that the Content is aware of.
func (c *Content) removeUnknownTags(results *result.List) error {
	*results = results.TransformTags(func(t result.Tags) result.Tags {
		filtered := result.NewTags()
		for tag := range t {
			if _, ok := c.Tags.ByName[tag]; ok {
				filtered.Add(tag)
			}
		}
		return filtered
	})
	return nil
}

// reduceTagsToMostExplicitOnly modifies the given results argument in place
// so that all contained results' tag sets only contain the most explicit tags
// based on the known tag sets contained within the Content.
func (c *Content) reduceTagsToMostExplicitOnly(results *result.List) error {
	for i, res := range *results {
		res.Tags = c.Tags.RemoveLowerPriorityTags(res.Tags)
		(*results)[i] = res
	}
	return nil
}

// addExpectationsToMutableChunk adds expectations for the results contained
// within |results| to the one mutable chunk that  should be in the Content.
// If not found, a new one will be created at the end of the Content.
func (c *Content) addExpectationsToMutableChunk(results *result.List) error {
	// Find the mutable chunk.
	// Chunks are considered immutable by default, unless annotated as
	// ROLLER_AUTOGENERATED_FAILURES.
	mutableTokens := []string{
		ROLLER_AUTOGENERATED_FAILURES,
	}

	// Bin the chunks into those that contain any of the strings in
	// mutableTokens in the comments and those that do not have these strings.
	immutableChunkIndicies, mutableChunkIndices := []int{}, []int{}
	for i, chunk := range c.Chunks {
		immutable := true

	comments:
		for _, line := range chunk.Comments {
			for _, token := range mutableTokens {
				if strings.Contains(line, token) {
					immutable = false
					break comments
				}
			}
		}

		if immutable {
			immutableChunkIndicies = append(immutableChunkIndicies, i)
		} else {
			mutableChunkIndices = append(mutableChunkIndices, i)
		}
	}

	var chunkToModify *Chunk
	if len(mutableChunkIndices) > 1 {
		return fmt.Errorf("Expected 1 mutable chunk, found %d", len(mutableChunkIndices))
	} else if len(mutableChunkIndices) == 0 {
		newChunk := Chunk{}
		newChunk.Comments = []string{
			"################################################################################",
			"# Autogenerated Failure expectations. Please triage.",
			ROLLER_AUTOGENERATED_FAILURES,
			"################################################################################",
		}
		c.Chunks = append(c.Chunks, newChunk)
		chunkToModify = &(c.Chunks[len(c.Chunks)-1])
	} else {
		chunkToModify = &(c.Chunks[mutableChunkIndices[0]])
	}

	// Add the new expectations to the mutable chunk.
	for _, res := range *results {
		expectation := Expectation{
			Bug:   "crbug.com/0000",
			Tags:  res.Tags,
			Query: res.Query.ExpectationFileString(),
			Status: []string{
				"Failure",
			},
		}
		chunkToModify.Expectations = append(chunkToModify.Expectations, expectation)
	}

	// Sort the mutable chunk's expectations.
	chunkToModify.Expectations.SortPrioritizeQuery()
	return nil
}

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
// TODO(crbug.com/371501714): Remove the Diagnostics return value since it is
// no longer used with the removal of commenting on CLs.
func (c *Content) Update(results result.List, testlist []query.Query, verbose bool) (Diagnostics, error) {
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

	if verbose {
		fmt.Println("result variants:")
		for i, tags := range variants {
			fmt.Printf(" (%.2d) %v\n", i, tags.List())
		}
	}

	// Add 'consumed' results for tests that were skipped.
	// This ensures that skipped results are not included in reduced trees.
	results = c.appendConsumedResultsForSkippedTests(results, testlist, variants)

	var pb *progressbar.ProgressBar
	if isatty.IsTerminal(os.Stdout.Fd()) || isatty.IsCygwinTerminal(os.Stderr.Fd()) {
		pb = progressbar.New(os.Stdout, nil)
		defer pb.Stop()
	}

	testQueryTree, _ := query.NewTree[struct{}]()
	for _, query := range testlist {
		testQueryTree.Add(query, struct{}{})
	}

	u := updater{
		in:              *c,
		out:             Content{},
		resultQueryTree: buildResultQueryTree(results),
		testQueryTree:   testQueryTree,
		variants:        variants,
		tagSets:         tagSets,
		pb:              pb,
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
	in              Content         // the original expectations Content
	out             Content         // newly built expectations Content
	resultQueryTree resultQueryTree // the results query tree
	testQueryTree   query.Tree[struct{}]
	variants        []container.Set[string]
	diags           []Diagnostic             // diagnostics raised during update
	tagSets         []result.Tags            // reverse-ordered tag-sets of 'in'
	pb              *progressbar.ProgressBar // Progress bar, may be nil
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
					if query := qd.Query.String(); !resultsForVariant.Contains(query) {
						resultsForVariant.Add(query)
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
	newFlakesComment = "# New flakes. Please triage - will be discarded/regenerated by the next roll:"
	// Chunk comment for new failures
	newFailuresComment = "# New failures. Please triage - will be discarded/regenerated by the next roll:"
	// Chunk comment for expectations the roller is allowed to mutate
	ROLLER_MUTABLE = "# ##ROLLER_MUTABLE##"
	// Chunk comment for expectations the roller should discard and rewrite
	ROLLER_DISCARD_AND_REWRITE = "# ##ROLLER_DISCARD_AND_REWRITE##"
	// Chunk comment for the AddExpectationsForFailingResults path.
	ROLLER_AUTOGENERATED_FAILURES = "# ##ROLLER_AUTOGENERATED_FAILURES##"
)

// resultQueryTree holds tree of queries to all results (no filtering by tag or
// status). The resultQueryTree is used to glob all the results that match a
// particular query.
type resultQueryTree struct {
	// All the results.
	results result.List
	// consumedAt is a list of line numbers for the i'th result in 'results'
	// Initially all line numbers are 0. When a result is consumed the line
	// number is set.
	consumedAt []int
	// Each tree node holds a list of indices to results.
	tree query.Tree[[]int]
}

// buildResultQueryTree builds the queryTree from the list of results.
func buildResultQueryTree(results result.List) resultQueryTree {
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
	return resultQueryTree{results, consumedAt, tree}
}

// glob returns the list of results matching the given tags under (or with) the
// given query.
func (qt *resultQueryTree) glob(q query.Query) (result.List, error) {
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
func (qt *resultQueryTree) globTags(q query.Query, t result.Tags) (result.List, error) {
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
func (qt *resultQueryTree) markAsConsumed(q query.Query, t result.Tags, line int) {
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

			glob, err := u.resultQueryTree.tree.Glob(q)
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
					if u.resultQueryTree.results[idx].Tags.ContainsAll(ex.Tags) {
						u.resultQueryTree.results[idx].Status = result.RetryOnFailure
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

	// Chunks are considered immutable by default, unless annotated as
	// ROLLER_MUTABLE or ROLLER_DISCARD_AND_REWRITE.
	mutableTokens := []string{
		ROLLER_MUTABLE,
		ROLLER_DISCARD_AND_REWRITE,
	}

	// Bin the chunks into those that contain any of the strings in
	// mutableTokens in the comments and those that do not have these strings.
	immutableChunks, mutableChunks := []Chunk{}, []Chunk{}
	for _, chunk := range u.in.Chunks {
		keep := true

	comments:
		for _, line := range chunk.Comments {
			for _, token := range mutableTokens {
				if strings.Contains(line, token) {
					keep = false
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

	// Skip over ROLLER_DISCARD_AND_REWRITE chunks (untriaged failures/flakes).
	// We'll just rebuild them at the end.
	for _, line := range in.Comments {
		if strings.HasPrefix(line, ROLLER_DISCARD_AND_REWRITE) {
			return Chunk{}
		}
	}

	// Build the new chunk's expectations
	newExpectations := container.NewMap[string, Expectation]()
	for _, exIn := range in.Expectations {
		if u.pb != nil {
			u.pb.Update(progressbar.Status{Total: progress.totalExpectations, Segments: []progressbar.Segment{
				{Count: 1 + progress.currentExpectation},
			}})
			progress.currentExpectation++
		}

		u.addExpectations(newExpectations, exIn, isImmutable)
	}

	// Sort the expectations to keep things clean and tidy.
	out := Chunk{Comments: in.Comments, Expectations: newExpectations.Values()}
	out.Expectations.Sort()
	return out
}

// expectation returns a new list of Expectations, based on the Expectation 'in',
// using the new result data.
func (u *updater) addExpectations(out container.Map[string, Expectation], in Expectation, isImmutable bool) {
	q := query.Parse(in.Query)

	// keyOf returns the map key for out
	keyOf := func(e Expectation) string { return fmt.Sprint(e.Tags, e.Query, e.Status) }

	// noResults is a helper for returning when the expectation has no test results.
	noResults := func() {
		if glob, err := u.testQueryTree.Glob(q); err == nil && len(glob) > 0 {
			// At least one test is found with the query in the test list - likely a variant that is not being run.
			if len(in.Tags) > 0 {
				u.diag(Note, in.Line, "no results found for query '%v' with tags %v", in.Query, in.Tags)
			} else {
				u.diag(Note, in.Line, "no results found for query '%v'", in.Query)
			}
			// Preserve.
			out.Add(keyOf(in), in)
		} else {
			// Remove the no-results expectation (do not add to out)
			u.diag(Warning, in.Line, "no tests exist with query '%v' - removing", in.Query)
		}
	}

	// Glob the results for the expectation's query + tag combination.
	// Ensure that none of these are already consumed.
	results, err := u.resultQueryTree.globTags(q, in.Tags)
	// If we can't find any results for this query + tag combination, then bail.
	switch {
	case errors.As(err, &query.ErrNoDataForQuery{}):
		noResults()
		return
	case err != nil:
		u.diag(Error, in.Line, "%v", err)
		return
	case len(results) == 0:
		noResults()
		return
	}

	// Before returning, mark all the results as consumed.
	// Note: this has to happen *after* we've generated the new expectations, as
	// marking the results as 'consumed' will impact the logic of
	// expectationsForRoot()
	defer u.resultQueryTree.markAsConsumed(q, in.Tags, in.Line)

	if isImmutable { // Expectation chunk was marked with 'KEEP'
		// Add a diagnostic if all tests of the expectation were 'Pass'
		if s := results.Statuses(); len(s) == 1 && s.One() == result.Pass {
			u.diagAllPass(in.Line, results)
		}
		out.Add(keyOf(in), in)
		return
	}

	// Rebuild the expectations for this query.
	expectations, somePass, someConsumed := u.expectationsForRoot(q, in.Line, in.Bug, in.Comment)

	// Add the new expectations to out
	for _, expectation := range expectations {
		out.Add(keyOf(expectation), expectation)
	}

	// Add a diagnostic if the expectation is filtered away
	if !out.Contains(keyOf(in)) && len(expectations) == 0 {
		switch {
		case somePass && someConsumed:
			u.diag(Note, in.Line, "expectation is partly covered by previous expectations and the remaining tests all pass")
		case someConsumed:
			u.diag(Note, in.Line, "expectation is fully covered by previous expectations")
		case somePass:
			u.diagAllPass(in.Line, results)
		}
	}

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
		filtered := u.resultQueryTree.results.FilterByVariant(variant)
		tree, err := filtered.StatusTree()
		if err != nil {
			return fmt.Errorf("while building tree for tags '%v': %w", variant, err)
		}
		// Reduce the tree.
		tree.Reduce(treeReducer)
		// Add all the reduced leaf nodes to 'roots'.
		for _, qd := range tree.List() {
			if qd.Data != result.Pass {
				roots.Add(qd.Query, true)
			}
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
		rootExpectations, _, _ := u.expectationsForRoot(
			root.Query,            // Root query
			0,                     // Line number
			"crbug.com/dawn/0000", // Bug
			"",                    // Comment
		)
		expectations = append(expectations, rootExpectations...)
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
		results Expectations
		comment string
	}{
		{flakes, newFlakesComment},
		{failures, newFailuresComment},
	} {
		if len(group.results) > 0 {
			group.results.Sort()
			u.out.Chunks = append(u.out.Chunks, Chunk{
				Comments: []string{
					"################################################################################",
					group.comment,
					ROLLER_DISCARD_AND_REWRITE,
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
) (
	expectations []Expectation, // The output expectations
	somePass bool, // Some of the results for the query had a Pass status
	someConsumed bool, // The query was at least partly consumed by previous expectations
) {
	results, err := u.resultQueryTree.glob(root)
	if err != nil {
		u.diag(Error, line, "%v", err)
		return nil, false, false
	}

	// Using the full list of unfiltered tests, generate the minimal set of
	// variants (tags) that uniquely classify the results with differing status.
	minimalVariants := u.
		removeUnknownTags(results).
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
	filtered := result.List{}
	for _, r := range reduced {
		switch r.Status {
		case result.Pass:
			somePass = true
		case consumed:
			someConsumed = true
		default:
			filtered = append(filtered, r)
		}
	}

	// Mark all the new expectation results as consumed.
	for _, r := range filtered {
		u.resultQueryTree.markAsConsumed(r.Query, r.Tags, 0)
	}

	// Transform the results to expectations.
	expectations = u.resultsToExpectations(filtered, bug, comment)
	return expectations, somePass, someConsumed
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

	out := make([]Expectation, 0, len(results))
	addedExpectations := container.NewSet[string]()
	for _, r := range results {
		q := r.Query.String()
		if r.Query.Target() == query.Tests && !r.Query.IsWildcard() {
			// The expectation validator wants a trailing ':' for test queries
			q += query.TargetDelimiter
		}
		e := Expectation{
			Bug:     bug,
			Tags:    u.in.Tags.RemoveLowerPriorityTags(r.Tags),
			Query:   q,
			Status:  []string{string(r.Status)},
			Comment: comment,
		}
		key := e.AsExpectationFileString()
		// We keep track of all expectations we've added so far to avoid cases where
		// two distinct results create the same expectation due to
		// RemoveLowerPriorityTags removing the distinguishing tags.
		if !addedExpectations.Contains(key) {
			out = append(out, e)
			addedExpectations.Add(key)
		}
	}

	return out
}

// removeUnknownTags returns a copy of the provided results with all tags not
// found in the expectations list removed
func (u *updater) removeUnknownTags(results result.List) result.List {
	return results.TransformTags(func(t result.Tags) result.Tags {
		filtered := result.NewTags()
		for tag := range t {
			if _, ok := u.in.Tags.ByName[tag]; ok {
				filtered.Add(tag)
			}
		}
		return filtered
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

// diagAllPass appends a new note diagnostic that all the tests now pass
func (u *updater) diagAllPass(line int, results result.List) {
	if c := len(results); c > 1 {
		u.diag(Note, line, "all %d tests now pass", len(results))
	} else {
		u.diag(Note, line, "test now passes")
	}
}
