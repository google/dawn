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

package roll

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"text/tabwriter"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/gitiles"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"go.chromium.org/luci/auth"
	"go.chromium.org/luci/auth/client/authcli"
)

func init() {
	common.Register(&cmd{})
}

const (
	depsRelPath      = "DEPS"
	tsSourcesRelPath = "third_party/gn/webgpu-cts/ts_sources.txt"
	refMain          = "refs/heads/main"
	noExpectations   = `# Clear all expectations to obtain full list of results`
)

type rollerFlags struct {
	gitPath  string
	tscPath  string
	auth     authcli.Flags
	cacheDir string
	rebuild  bool // Rebuild the expectations file from scratch
	preserve bool // If false, abandon past roll changes
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
	tscPath, _ := exec.LookPath("tsc")
	c.flags.auth.Register(flag.CommandLine, common.DefaultAuthOptions())
	flag.StringVar(&c.flags.gitPath, "git", gitPath, "path to git")
	flag.StringVar(&c.flags.tscPath, "tsc", tscPath, "path to tsc")
	flag.StringVar(&c.flags.cacheDir, "cache", common.DefaultCacheDir, "path to the results cache")
	flag.BoolVar(&c.flags.rebuild, "rebuild", false, "rebuild the expectation file from scratch")
	flag.BoolVar(&c.flags.preserve, "preserve", false, "do not abandon existing rolls")

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
		name, path string
	}{
		{name: "git", path: c.flags.gitPath},
		{name: "tsc", path: c.flags.tscPath},
	} {
		if _, err := os.Stat(tool.path); err != nil {
			return fmt.Errorf("failed to find path to %v: %v", tool.name, err)
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
	gerrit, err := gerrit.New(cfg.Gerrit.Host, gerrit.Credentials{})
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
		cfg:      cfg,
		flags:    c.flags,
		auth:     auth,
		bb:       bb,
		rdb:      rdb,
		git:      git,
		gerrit:   gerrit,
		chromium: chromium,
		dawn:     dawn,
		ctsDir:   ctsDir,
	}
	return r.roll(ctx)
}

type roller struct {
	cfg      common.Config
	flags    rollerFlags
	auth     auth.Options
	bb       *buildbucket.Buildbucket
	rdb      *resultsdb.ResultsDB
	git      *git.Git
	gerrit   *gerrit.Gerrit
	chromium *gitiles.Gitiles
	dawn     *gitiles.Gitiles
	ctsDir   string
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
	if updatedDEPS == "" {
		// Already up to date
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
	ex, err := expectations.Parse(expectationsFile)
	if err != nil {
		return fmt.Errorf("failed to load expectations: %v", err)
	}

	// If the user requested a full rebuild of the expecations, strip out
	// everything but comment chunks.
	if r.flags.rebuild {
		rebuilt := ex.Clone()
		rebuilt.Chunks = rebuilt.Chunks[:0]
		for _, c := range ex.Chunks {
			switch {
			case c.IsBlankLine():
				rebuilt.MaybeAddBlankLine()
			case c.IsCommentOnly():
				rebuilt.Chunks = append(rebuilt.Chunks, c)
			}
		}
		ex = rebuilt
	}

	// Regenerate the typescript dependency list
	tsSources, err := r.genTSDepList(ctx)
	if err != nil {
		return fmt.Errorf("failed to generate ts_sources.txt: %v", err)
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
	if len(existingRolls) == 0 {
		msg := r.rollCommitMessage(oldCTSHash, newCTSHash, ctsLog, "")
		change, err := r.gerrit.CreateChange(r.cfg.Gerrit.Project, "main", msg, true)
		if err != nil {
			return err
		}
		changeID = change.ID
		log.Printf("created gerrit change %v...", change.Number)
	} else {
		changeID = existingRolls[0].ID
		log.Printf("reusing existing gerrit change %v...", existingRolls[0].Number)
	}

	// Update the DEPS, and ts-sources file.
	// Update the expectations with the re-formatted content, and updated
	//timestamp.
	updateExpectationUpdateTimestamp(&ex)
	msg := r.rollCommitMessage(oldCTSHash, newCTSHash, ctsLog, changeID)
	ps, err := r.gerrit.EditFiles(changeID, msg, map[string]string{
		depsRelPath:                     updatedDEPS,
		common.RelativeExpectationsPath: ex.String(),
		tsSourcesRelPath:                tsSources,
	})
	if err != nil {
		return fmt.Errorf("failed to update change '%v': %v", changeID, err)
	}

	// Begin main roll loop
	const maxAttempts = 3
	results := result.List{}
	for attempt := 0; ; attempt++ {
		// Kick builds
		log.Printf("building (attempt %v)...\n", attempt)
		builds, err := common.GetOrStartBuildsAndWait(ctx, r.cfg, ps, r.bb, false)
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
		diags, err := newExpectations.Update(results)
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
		})
		if err != nil {
			return fmt.Errorf("failed to update change '%v': %v", changeID, err)
		}

		if attempt >= maxAttempts {
			err := fmt.Errorf("CTS failed after %v attempts.\nGiving up", attempt)
			r.gerrit.Comment(ps, err.Error(), nil)
			return err
		}
	}

	if err := r.gerrit.SetReadyForReview(changeID, "CTS roll succeeded"); err != nil {
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
	msg.WriteString("Update expectations and ts_sources")
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
		fc[i] = gerrit.FileComment{
			Path:    common.RelativeExpectationsPath,
			Side:    gerrit.Left,
			Line:    d.Line,
			Message: fmt.Sprintf("%v: %v", d.Severity, d.Message),
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
	repo, err := r.git.Clone(dir, host, nil)
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
func (r *roller) genTSDepList(ctx context.Context) (string, error) {
	cmd := exec.CommandContext(ctx, r.flags.tscPath, "--project",
		filepath.Join(r.ctsDir, "tsconfig.json"),
		"--listFiles",
		"--declaration", "false",
		"--sourceMap", "false")
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
