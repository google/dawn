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

package roll

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"sync"
	"text/tabwriter"
	"time"

	commonAuth "dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/gitiles"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"go.chromium.org/luci/auth"
	"go.chromium.org/luci/auth/client/authcli"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

func init() {
	common.Register(&cmd{})
}

const (
	depsRelPath          = "DEPS"
	gitLinkPath          = "third_party/webgpu-cts"
	tsSourcesRelPath     = "third_party/gn/webgpu-cts/ts_sources.txt"
	testListRelPath      = "third_party/gn/webgpu-cts/test_list.txt"
	cacheListRelPath     = "third_party/gn/webgpu-cts/cache_list.txt"
	cacheTarGz           = "third_party/gn/webgpu-cts/cache.tar.gz"
	resourceFilesRelPath = "third_party/gn/webgpu-cts/resource_files.txt"
	webTestsPath         = "webgpu-cts/webtests"
	refMain              = "refs/heads/main"
	noExpectations       = `# Clear all expectations to obtain full list of results`
)

type rollerFlags struct {
	gitPath             string
	npmPath             string
	nodePath            string
	auth                authcli.Flags
	cacheDir            string
	force               bool // Create a new roll, even if CTS is up to date
	rebuild             bool // Rebuild the expectations file from scratch
	preserve            bool // If false, abandon past roll changes
	sendToGardener      bool // If true, automatically send to the gardener for review
	parentSwarmingRunID string
	maxAttempts         int
}

type cmd struct {
	flags rollerFlags
}

func (cmd) Name() string {
	return "roll"
}

func (cmd) Desc() string {
	return "roll CTS and re-generate expectations"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	gitPath, _ := exec.LookPath("git")
	npmPath, _ := exec.LookPath("npm")
	c.flags.auth.Register(flag.CommandLine, commonAuth.DefaultAuthOptions( /* needsCloudScopes */ true))
	flag.StringVar(&c.flags.gitPath, "git", gitPath, "path to git")
	flag.StringVar(&c.flags.npmPath, "npm", npmPath, "path to npm")
	flag.StringVar(&c.flags.nodePath, "node", fileutils.NodePath(), "path to node")
	flag.StringVar(&c.flags.cacheDir, "cache", common.DefaultCacheDir, "path to the results cache")
	flag.BoolVar(&c.flags.force, "force", false, "create a new roll, even if CTS is up to date")
	flag.BoolVar(&c.flags.rebuild, "rebuild", false, "rebuild the expectation file from scratch")
	flag.BoolVar(&c.flags.preserve, "preserve", false, "do not abandon existing rolls")
	flag.BoolVar(&c.flags.sendToGardener, "send-to-gardener", false, "send the CL to the WebGPU gardener for review")
	flag.StringVar(&c.flags.parentSwarmingRunID, "parent-swarming-run-id", "", "parent swarming run id. All triggered tasks will be children of this task and will be canceled if the parent is canceled.")
	flag.IntVar(&c.flags.maxAttempts, "max-attempts", 3, "number of update attempts before giving up")
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Validate command line arguments
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	// Check tools can be found
	for _, tool := range []struct {
		name, path, hint string
	}{
		{name: "git", path: c.flags.gitPath},
		{name: "npm", path: c.flags.npmPath},
		{name: "node", path: c.flags.nodePath},
	} {
		if _, err := os.Stat(tool.path); err != nil {
			return fmt.Errorf("failed to find path to %v: %v. %v", tool.name, err, tool.hint)
		}
	}

	// Create a temporary directory for local checkouts
	tmpDir, err := os.MkdirTemp("", "dawn-cts-roll")
	if err != nil {
		return err
	}
	defer os.RemoveAll(tmpDir)
	ctsDir := filepath.Join(tmpDir, "cts")

	// Create the various service clients
	git, err := git.New(c.flags.gitPath)
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}
	gerrit, err := gerrit.New(ctx, auth, cfg.Gerrit.Host)
	if err != nil {
		return err
	}
	chromium, err := gitiles.New(ctx, cfg.Git.CTS.Host, cfg.Git.CTS.Project)
	if err != nil {
		return err
	}
	dawn, err := gitiles.New(ctx, cfg.Git.Dawn.Host, cfg.Git.Dawn.Project)
	if err != nil {
		return err
	}
	bb, err := buildbucket.New(ctx, auth)
	if err != nil {
		return err
	}
	rdb, err := resultsdb.New(ctx, auth)
	if err != nil {
		return err
	}

	// Construct the roller, and roll
	r := roller{
		cfg:                 cfg,
		flags:               c.flags,
		auth:                auth,
		bb:                  bb,
		parentSwarmingRunID: c.flags.parentSwarmingRunID,
		rdb:                 rdb,
		git:                 git,
		gerrit:              gerrit,
		chromium:            chromium,
		dawn:                dawn,
		ctsDir:              ctsDir,
	}
	return r.roll(ctx)
}

type roller struct {
	cfg                 common.Config
	flags               rollerFlags
	auth                auth.Options
	bb                  *buildbucket.Buildbucket
	parentSwarmingRunID string
	rdb                 *resultsdb.ResultsDB
	git                 *git.Git
	gerrit              *gerrit.Gerrit
	chromium            *gitiles.Gitiles
	dawn                *gitiles.Gitiles
	ctsDir              string
}

func (r *roller) roll(ctx context.Context) error {
	// Fetch the latest Dawn main revision
	dawnHash, err := r.dawn.Hash(ctx, refMain)
	if err != nil {
		return err
	}

	// Update the DEPS file
	updatedDEPS, newCTSHash, oldCTSHash, err := r.updateDEPS(ctx, dawnHash)
	if err != nil {
		return err
	}
	if newCTSHash == oldCTSHash && !r.flags.force {
		// Already up to date
		fmt.Println("CTS is already up to date")
		return nil
	}

	log.Printf("starting CTS roll from %v to %v...", oldCTSHash[:8], newCTSHash[:8])

	// Checkout the CTS at the latest revision
	ctsRepo, err := r.checkout("cts", r.ctsDir, r.cfg.Git.CTS.HttpsURL(), newCTSHash)
	if err != nil {
		return err
	}

	// Fetch the log of changes between last roll and now
	ctsLog, err := ctsRepo.Log(&git.LogOptions{From: oldCTSHash, To: newCTSHash})
	if err != nil {
		return err
	}
	ctsLog = ctsLog[:len(ctsLog)-1] // Don't include the oldest change in the log

	// Download and parse the expectations file
	expectationsFile, err := r.dawn.DownloadFile(ctx, refMain, common.RelativeExpectationsPath)
	if err != nil {
		return err
	}
	ex, err := expectations.Parse(common.RelativeExpectationsPath, expectationsFile)
	if err != nil {
		return fmt.Errorf("failed to load expectations: %v", err)
	}

	// If the user requested a full rebuild of the expectations, strip out
	// everything but comment chunks.
	if r.flags.rebuild {
		rebuilt := ex.Clone()
		rebuilt.Chunks = rebuilt.Chunks[:0]
		for _, c := range ex.Chunks {
			if c.IsCommentOnly() {
				rebuilt.Chunks = append(rebuilt.Chunks, c)
			}
		}
		ex = rebuilt
	}

	generatedFiles, err := r.generateFiles(ctx)
	if err != nil {
		return err
	}

	// Pull out the test list from the generated files
	testlist := func() []query.Query {
		lines := strings.Split(generatedFiles[testListRelPath], "\n")
		list := make([]query.Query, len(lines))
		for i, line := range lines {
			list[i] = query.Parse(line)
		}
		return list
	}()

	deletedFiles := []string{}
	if currentWebTestFiles, err := r.dawn.ListFiles(ctx, dawnHash, webTestsPath); err != nil {
		// If there's an error, allow NotFound. It means the directory did not exist, so no files
		// need to be deleted.
		if e, ok := status.FromError(err); !ok || e.Code() != codes.NotFound {
			return fmt.Errorf("listing current web tests failed: %v", err)
		}

		for _, f := range currentWebTestFiles {
			// If the file is not generated in this revision, and it is an .html file,
			// mark it for deletion.
			if !strings.HasSuffix(f, ".html") {
				continue
			}
			if _, exists := generatedFiles[f]; !exists {
				deletedFiles = append(deletedFiles, f)
			}
		}
	}

	// Look for an existing gerrit change to update
	existingRolls, err := r.findExistingRolls()
	if err != nil {
		return err
	}

	// Abandon existing rolls, if -preserve is false
	if !r.flags.preserve && len(existingRolls) > 0 {
		log.Printf("abandoning %v existing roll...", len(existingRolls))
		for _, change := range existingRolls {
			if err := r.gerrit.Abandon(change.ChangeID); err != nil {
				return err
			}
		}
		existingRolls = nil
	}

	// Create a new gerrit change, if needed
	changeID := ""
	if r.flags.preserve || len(existingRolls) == 0 {
		msg := r.rollCommitMessage(oldCTSHash, newCTSHash, ctsLog, "")
		change, err := r.gerrit.CreateChange(r.cfg.Gerrit.Project, "main", msg, true)
		if err != nil {
			return err
		}
		changeID = change.ID
		log.Printf("created gerrit change %v (%v)...", change.Number, change.URL)
	} else {
		changeID = existingRolls[0].ID
		log.Printf("reusing existing gerrit change %v (%v)...", existingRolls[0].Number, existingRolls[0].URL)
	}

	// Update the DEPS, expectations, and other generated files.
	updateExpectationUpdateTimestamp(&ex)
	generatedFiles[depsRelPath] = updatedDEPS
	generatedFiles[gitLinkPath] = newCTSHash
	generatedFiles[common.RelativeExpectationsPath] = ex.String()

	msg := r.rollCommitMessage(oldCTSHash, newCTSHash, ctsLog, changeID)
	ps, err := r.gerrit.EditFiles(changeID, msg, generatedFiles, deletedFiles)
	if err != nil {
		return fmt.Errorf("failed to update change '%v': %v", changeID, err)
	}

	// Begin main roll loop
	results := result.List{}
	for attempt := 0; ; attempt++ {
		// Kick builds
		log.Printf("building (attempt %v)...\n", attempt)
		builds, err := common.GetOrStartBuildsAndWait(ctx, r.cfg, ps, r.bb, r.parentSwarmingRunID, false)
		if err != nil {
			return err
		}

		// Look to see if any of the builds failed
		failingBuilds := []string{}
		for id, build := range builds {
			if build.Status != buildbucket.StatusSuccess {
				failingBuilds = append(failingBuilds, id)
			}
		}
		if len(failingBuilds) > 0 {
			sort.Strings(failingBuilds)
			log.Println("builds failed: ", failingBuilds)
		}

		// Gather the build results
		log.Println("gathering results...")
		psResults, err := common.CacheResults(ctx, r.cfg, ps, r.flags.cacheDir, r.rdb, builds)
		if err != nil {
			return err
		}

		// Merge the new results into the accumulated results
		log.Println("merging results...")
		results = result.Merge(results, psResults)

		// Rebuild the expectations with the accumulated results
		log.Println("building new expectations...")
		// Note: The new expectations are not used if the last attempt didn't
		// fail, but we always want to post the diagnostics
		newExpectations := ex.Clone()
		diags, err := newExpectations.Update(results, testlist)
		if err != nil {
			return err
		}

		// Post statistics and expectation diagnostics
		log.Println("posting stats & diagnostics...")
		if err := r.postComments(ps, diags, results); err != nil {
			return err
		}

		// If all the builds attempted, then we're done!
		if len(failingBuilds) == 0 {
			break
		}

		// Otherwise, push the updated expectations, and try again
		log.Println("updating expectations...")
		updateExpectationUpdateTimestamp(&newExpectations)
		ps, err = r.gerrit.EditFiles(changeID, msg, map[string]string{
			common.RelativeExpectationsPath: newExpectations.String(),
		}, nil)
		if err != nil {
			return fmt.Errorf("failed to update change '%v': %v", changeID, err)
		}

		if attempt >= r.flags.maxAttempts {
			err := fmt.Errorf("CTS failed after %v attempts.\nGiving up", attempt)
			r.gerrit.Comment(ps, err.Error(), nil)
			return err
		}
	}

	reviewer := ""
	if r.flags.sendToGardener {
		resp, err := http.Get("https://chrome-ops-rotation-proxy.appspot.com/current/grotation:webgpu-gardener")
		if err != nil {
			return err
		}
		defer resp.Body.Close()

		jsonResponse, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			return err
		}

		type StructuredJSONResponse struct {
			Emails []string
		}
		var jsonRes StructuredJSONResponse
		if err := json.Unmarshal(jsonResponse, &jsonRes); err != nil {
			return err
		}
		if len(jsonRes.Emails) < 1 {
			return fmt.Errorf("Expected at least one email in JSON response %s", jsonRes)
		}
		reviewer = jsonRes.Emails[0]
	}

	if err := r.gerrit.SetReadyForReview(changeID, "CTS roll succeeded", reviewer); err != nil {
		return fmt.Errorf("failed to mark change as ready for review: %v", err)
	}

	return nil
}

// Updates the '# Last rolled:' string in the expectations file.
func updateExpectationUpdateTimestamp(content *expectations.Content) {
	prefix := "# Last rolled: "
	comment := prefix + time.Now().UTC().Format("2006-01-02 03:04:05PM")
	for _, chunk := range content.Chunks {
		for l, line := range chunk.Comments {
			if strings.HasPrefix(line, prefix) {
				chunk.Comments[l] = comment
				return
			}
		}
	}
	newChunks := []expectations.Chunk{}
	if len(content.Chunks) > 0 {
		newChunks = append(newChunks,
			content.Chunks[0],
			expectations.Chunk{},
		)
	}
	newChunks = append(newChunks,
		expectations.Chunk{Comments: []string{comment}},
	)
	if len(content.Chunks) > 0 {
		newChunks = append(newChunks, content.Chunks[1:]...)
	}

	content.Chunks = newChunks
}

// rollCommitMessage returns the commit message for the roll
func (r *roller) rollCommitMessage(
	oldCTSHash, newCTSHash string,
	ctsLog []git.CommitInfo,
	changeID string) string {

	msg := &strings.Builder{}
	msg.WriteString(common.RollSubjectPrefix)
	msg.WriteString(oldCTSHash[:9])
	msg.WriteString("..")
	msg.WriteString(newCTSHash[:9])
	msg.WriteString(" (")
	msg.WriteString(strconv.Itoa(len(ctsLog)))
	if len(ctsLog) == 1 {
		msg.WriteString(" commit)")
	} else {
		msg.WriteString(" commits)")
	}
	msg.WriteString("\n\n")
	msg.WriteString("Regenerated:\n")
	msg.WriteString(" - expectations.txt\n")
	msg.WriteString(" - ts_sources.txt\n")
	msg.WriteString(" - test_list.txt\n")
	msg.WriteString(" - cache_list.txt\n")
	msg.WriteString(" - resource_files.txt\n")
	msg.WriteString(" - webtest .html files\n")
	msg.WriteString("\n\n")
	msg.WriteString("https://chromium.googlesource.com/external/github.com/gpuweb/cts/+log/")
	msg.WriteString(oldCTSHash[:12])
	msg.WriteString("..")
	msg.WriteString(newCTSHash[:12])
	msg.WriteString("\n")
	for _, change := range ctsLog {
		msg.WriteString(" - ")
		msg.WriteString(change.Hash.String()[:6])
		msg.WriteString(" ")
		msg.WriteString(change.Subject)
		msg.WriteString("\n")
	}
	msg.WriteString("\n")
	msg.WriteString("Created with './tools/run cts roll'")
	msg.WriteString("\n")
	msg.WriteString("\n")
	if len(r.cfg.Builders) > 0 {
		msg.WriteString("Cq-Include-Trybots: ")
		buildersByBucket := container.NewMap[string, []string]()
		for _, build := range r.cfg.Builders {
			key := fmt.Sprintf("luci.%v.%v", build.Project, build.Bucket)
			buildersByBucket[key] = append(buildersByBucket[key], build.Builder)
		}
		first := true
		for _, bucket := range buildersByBucket.Keys() {
			// Cq-Include-Trybots: luci.chromium.try:win-dawn-rel;luci.dawn.try:mac-dbg,mac-rel
			if !first {
				msg.WriteString(";")
			}
			first = false
			msg.WriteString(bucket)
			msg.WriteString(":")
			builders := buildersByBucket[bucket]
			sort.Strings(builders)
			msg.WriteString(strings.Join(builders, ","))
		}
		msg.WriteString("\n")
	}
	msg.WriteString("Include-Ci-Only-Tests: true\n")
	if changeID != "" {
		msg.WriteString("Change-Id: ")
		msg.WriteString(changeID)
		msg.WriteString("\n")
	}
	return msg.String()
}

func (r *roller) postComments(ps gerrit.Patchset, diags []expectations.Diagnostic, results result.List) error {
	fc := make([]gerrit.FileComment, len(diags))
	for i, d := range diags {
		var prefix string
		switch d.Severity {
		case expectations.Error:
			prefix = "🟥"
		case expectations.Warning:
			prefix = "🟨"
		case expectations.Note:
			prefix = "🟦"
		}
		fc[i] = gerrit.FileComment{
			Path:    common.RelativeExpectationsPath,
			Side:    gerrit.Left,
			Line:    d.Line,
			Message: fmt.Sprintf("%v %v: %v", prefix, d.Severity, d.Message),
		}
	}

	sb := &strings.Builder{}

	{
		sb.WriteString("Tests by status:\n")
		counts := map[result.Status]int{}
		for _, r := range results {
			counts[r.Status] = counts[r.Status] + 1
		}
		type StatusCount struct {
			status result.Status
			count  int
		}
		statusCounts := []StatusCount{}
		for s, n := range counts {
			if n > 0 {
				statusCounts = append(statusCounts, StatusCount{s, n})
			}
		}
		sort.Slice(statusCounts, func(i, j int) bool { return statusCounts[i].status < statusCounts[j].status })
		sb.WriteString("```\n")
		tw := tabwriter.NewWriter(sb, 0, 1, 0, ' ', 0)
		for _, sc := range statusCounts {
			fmt.Fprintf(tw, "%v:\t %v\n", sc.status, sc.count)
		}
		tw.Flush()
		sb.WriteString("```\n")
	}
	{
		sb.WriteString("Top 25 slowest tests:\n")
		sort.Slice(results, func(i, j int) bool {
			return results[i].Duration > results[j].Duration
		})
		const N = 25
		topN := results
		if len(topN) > N {
			topN = topN[:N]
		}
		sb.WriteString("```\n")
		for i, r := range topN {
			fmt.Fprintf(sb, "%3.1d: %v\n", i, r)
		}
		sb.WriteString("```\n")
	}

	if err := r.gerrit.Comment(ps, sb.String(), fc); err != nil {
		return fmt.Errorf("failed to post stats on change: %v", err)
	}
	return nil
}

// findExistingRolls looks for all existing open CTS rolls by this user
func (r *roller) findExistingRolls() ([]gerrit.ChangeInfo, error) {
	// Look for an existing gerrit change to update
	changes, _, err := r.gerrit.QueryChanges("owner:me",
		"is:open",
		fmt.Sprintf(`repo:"%v"`, r.cfg.Git.Dawn.Project),
		fmt.Sprintf(`message:"%v"`, common.RollSubjectPrefix))
	if err != nil {
		return nil, fmt.Errorf("failed to find existing roll gerrit changes: %v", err)
	}
	return changes, nil
}

// checkout performs a git checkout of the repo at host to dir at the given hash
func (r *roller) checkout(project, dir, host, hash string) (*git.Repository, error) {
	log.Printf("cloning %v to '%v'...", project, dir)
	repo, err := r.git.Clone(dir, host, &git.CloneOptions{Timeout: time.Minute * 10})
	if err != nil {
		return nil, fmt.Errorf("failed to clone %v: %v", project, err)
	}
	log.Printf("checking out %v @ '%v'...", project, hash)
	if _, err := repo.Fetch(hash, nil); err != nil {
		return nil, fmt.Errorf("failed to fetch project %v @ %v: %v",
			project, hash, err)
	}
	if err := repo.Checkout(hash, nil); err != nil {
		return nil, fmt.Errorf("failed to checkout project %v @ %v: %v",
			project, hash, err)
	}
	return repo, nil
}

// Call 'npm ci' in the CTS directory, and generates a map of project-relative
// file path to file content for the CTS roll's change. This includes:
// * type-script source files
// * CTS test list
// * CTS cache list
// * resource file list
// * webtest file sources
func (r *roller) generateFiles(ctx context.Context) (map[string]string, error) {
	// Run 'npm ci' to fetch modules and tsc
	if err := common.InstallCTSDeps(ctx, r.ctsDir, r.flags.npmPath); err != nil {
		return nil, err
	}

	log.Printf("generating files for changelist...")

	// Run the below concurrently
	mutex := sync.Mutex{}
	files := map[string]string{} // guarded by mutex
	wg := sync.WaitGroup{}

	errs := make(chan error, 8)

	// Generate web tests HTML files
	wg.Add(1)
	go func() {
		defer wg.Done()
		if out, err := r.genWebTestSources(ctx); err == nil {
			mutex.Lock()
			defer mutex.Unlock()
			for file, content := range out {
				files[file] = content
			}
		} else {
			errs <- fmt.Errorf("failed to generate web tests: %v", err)
		}
	}()

	// Generate typescript sources list, test list, resources file list.
	for relPath, generator := range map[string]func(context.Context) (string, error){
		tsSourcesRelPath:     r.genTSDepList,
		testListRelPath:      r.genTestList,
		resourceFilesRelPath: r.genResourceFilesList,
		cacheListRelPath: func(context.Context) (string, error) {
			return common.BuildCache(ctx, r.ctsDir, r.flags.nodePath, r.flags.npmPath, r.flags.auth)
		},
	} {
		relPath, generator := relPath, generator // Capture values, not iterators
		wg.Add(1)
		go func() {
			defer wg.Done()
			if out, err := generator(ctx); err == nil {
				mutex.Lock()
				defer mutex.Unlock()
				files[relPath] = out
			} else {
				errs <- fmt.Errorf("failed to generate %v: %v", relPath, err)
			}
		}()
	}

	// Wait for all the above to complete
	wg.Wait()
	close(errs)

	// Check for errors
	for err := range errs {
		return nil, err
	}

	return files, nil
}

// updateDEPS fetches and updates the Dawn DEPS file at 'dawnRef' so that all
// CTS hashes are changed to the latest CTS hash.
func (r *roller) updateDEPS(ctx context.Context, dawnRef string) (newDEPS, newCTSHash, oldCTSHash string, err error) {
	newCTSHash, err = r.chromium.Hash(ctx, refMain)
	if err != nil {
		return "", "", "", err
	}
	deps, err := r.dawn.DownloadFile(ctx, dawnRef, depsRelPath)
	if err != nil {
		return "", "", "", err
	}
	newDEPS, oldCTSHash, err = common.UpdateCTSHashInDeps(deps, newCTSHash)
	if err != nil {
		return "", "", "", err
	}

	return newDEPS, newCTSHash, oldCTSHash, nil
}

// genTSDepList returns a list of source files, for the CTS checkout at r.ctsDir
// This list can be used to populate the ts_sources.txt file.
// Requires tsc to be found at './node_modules/.bin/tsc' in the CTS directory
// (e.g. must be called post 'npm ci')
func (r *roller) genTSDepList(ctx context.Context) (string, error) {
	tscPath := filepath.Join(r.ctsDir, "node_modules/.bin/tsc")
	if !fileutils.IsExe(tscPath) {
		return "", fmt.Errorf("tsc not found at '%v'", tscPath)
	}

	cmd := exec.CommandContext(ctx, tscPath, "--project",
		filepath.Join(r.ctsDir, "tsconfig.json"),
		"--listFiles",
		"--declaration", "false",
		"--sourceMap", "false")

	// Note: we're ignoring the error for this as tsc typically returns status 2.
	out, _ := cmd.Output()

	prefix := filepath.ToSlash(r.ctsDir) + "/"

	deps := []string{}
	for _, line := range strings.Split(string(out), "\n") {
		if strings.HasPrefix(line, prefix) {
			line = line[len(prefix):]
			if strings.HasPrefix(line, "src/") {
				deps = append(deps, line)
			}
		}
	}

	return strings.Join(deps, "\n") + "\n", nil
}

// genTestList returns the newline delimited list of test names, for the CTS checkout at r.ctsDir
func (r *roller) genTestList(ctx context.Context) (string, error) {
	// Run 'src/common/runtime/cmdline.ts' to obtain the full test list
	cmd := exec.CommandContext(ctx, r.flags.nodePath,
		"-e", "require('./src/common/tools/setup-ts-in-node.js');require('./src/common/runtime/cmdline.ts');",
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		"--list",
		"webgpu:*",
	)
	cmd.Dir = r.ctsDir

	stderr := bytes.Buffer{}
	cmd.Stderr = &stderr

	out, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("failed to generate test list: %w\n%v", err, stderr.String())
	}

	tests := []string{}
	for _, test := range strings.Split(string(out), "\n") {
		if test != "" {
			tests = append(tests, test)
		}
	}

	return strings.Join(tests, "\n"), nil
}

// genResourceFilesList returns a list of resource files, for the CTS checkout at r.ctsDir
// This list can be used to populate the resource_files.txt file.
func (r *roller) genResourceFilesList(ctx context.Context) (string, error) {
	dir := filepath.Join(r.ctsDir, "src", "resources")
	files, err := filepath.Glob(filepath.Join(dir, "*"))
	if err != nil {
		return "", err
	}
	for i, file := range files {
		file, err := filepath.Rel(dir, file)
		if err != nil {
			return "", err
		}
		files[i] = file
	}
	return strings.Join(files, "\n") + "\n", nil
}

// genWebTestSources returns a map of generated webtest file names to contents, for the CTS checkout at r.ctsDir
func (r *roller) genWebTestSources(ctx context.Context) (map[string]string, error) {
	generatedFiles := map[string]string{}
	htmlSearchDir := filepath.Join(r.ctsDir, "src", "webgpu")
	err := filepath.Walk(htmlSearchDir,
		func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}
			if !strings.HasSuffix(info.Name(), ".html") || info.IsDir() {
				return nil
			}
			relPath, err := filepath.Rel(htmlSearchDir, path)
			if err != nil {
				return err
			}

			data, err := os.ReadFile(path)
			if err != nil {
				return err
			}
			contents := string(data)

			// Find the index after the starting html tag.
			i := strings.Index(contents, "<html")
			i = i + strings.Index(contents[i:], ">")
			i = i + 1

			// Insert a base tag so the fetched resources will come from the generated CTS JavaScript sources.
			contents = contents[:i] + "\n" + `  <base ref="/gen/third_party/dawn/webgpu-cts/src/webgpu" />` + contents[i:]

			generatedFiles[filepath.Join(webTestsPath, relPath)] = contents
			return nil
		})
	if err != nil {
		return nil, err
	}
	return generatedFiles, nil
}
