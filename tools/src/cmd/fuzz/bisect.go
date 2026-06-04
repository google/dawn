// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.

type bisectConfig struct {
	*taskConfig
	failingHash string
	passingHash string
	isFix       bool
	fuzzerName  string
}

// runBisect orchestrates the automated bisection of a fuzzer testcase.
// It verifies that both passing and failing points behave as expected and then executes a git bisect run.
func runBisect(t *taskConfig) error {
	if t.bisectFile == "" {
		return fmt.Errorf("-bisect flag requires a test case file path")
	}
	if t.knownFailing == "" {
		return fmt.Errorf("-known-failing flag is required when bisecting")
	}

	bc := &bisectConfig{taskConfig: t}

	bc.fuzzerName = filepath.Base(t.fuzzer)

	// Verify build directory exists and is a GN build (contains args.gn)
	argsGnPath := filepath.Join(t.build, "args.gn")
	if !fileutils.IsFile(argsGnPath, t.osWrapper) {
		return fmt.Errorf("build directory '%v' does not appear to be a GN build directory (missing args.gn), which is required for bisecting", t.build)
	}

	var err error
	bc.failingHash, err = resolveToHash(bc.knownFailing, bc.execWrapper)
	if err != nil {
		return fmt.Errorf("failed to resolve known-failing '%s': %w", bc.knownFailing, err)
	}

	bc.passingHash, err = resolveToHash(bc.knownPassing, bc.execWrapper)
	if err != nil {
		return fmt.Errorf("failed to resolve known-passing '%s': %w", bc.knownPassing, err)
	}

	fmt.Printf("Resolved known-failing to commit: %s\n", bc.failingHash)
	fmt.Printf("Resolved known-passing to commit: %s\n", bc.passingHash)

	origRefBytes, err := bc.runCmd("git", "rev-parse", "--abbrev-ref", "HEAD")
	if err != nil {
		return fmt.Errorf("failed to get original HEAD reference: %w", err)
	}
	origRef := strings.TrimSpace(string(origRefBytes))
	if origRef == "HEAD" {
		origHeadBytes, err := bc.runCmd("git", "rev-parse", "HEAD")
		if err != nil {
			return fmt.Errorf("failed to get original HEAD commit: %w", err)
		}
		origRef = strings.TrimSpace(string(origHeadBytes))
	}

	// Ensure repository state restore upon completion
	defer func() {
		fmt.Printf("Restoring repository to original state (%s)...\n", origRef)
		_, _ = bc.runCmd("git", "checkout", origRef)
		_, _ = bc.runCmd("gclient", "sync")
	}()

	// Determine ordering
	mergeBaseBytes, err := bc.runCmd("git", "merge-base", bc.failingHash, bc.passingHash)
	if err != nil {
		return fmt.Errorf("failed to find merge-base between %s and %s: %w", bc.failingHash, bc.passingHash, err)
	}
	mergeBase := strings.TrimSpace(string(mergeBaseBytes))

	if mergeBase == bc.failingHash {
		bc.isFix = true
		fmt.Println("Detected flow: FAILING commit is an ancestor of PASSING commit (looking for the FIX).")
	} else if mergeBase == bc.passingHash {
		bc.isFix = false
		fmt.Println("Detected flow: PASSING commit is an ancestor of FAILING commit (looking for the BREAKAGE).")
	} else {
		return fmt.Errorf("the failing commit (%s) and passing commit (%s) are not in a direct ancestral relationship (merge-base is %s)", bc.failingHash, bc.passingHash, mergeBase)
	}

	// Verify failing commit behaves as expected
	fmt.Printf("Verifying known-failing commit (%s)...\n", bc.failingHash)
	if err := bc.verifyCommit(bc.failingHash, false); err != nil {
		return fmt.Errorf("verification of known-failing commit %s failed: %w", bc.failingHash, err)
	}
	fmt.Println("Verification succeeded: fuzzer failed on known-failing commit as expected.")

	// Verify passing commit behaves as expected
	fmt.Printf("Verifying known-passing commit (%s)...\n", bc.passingHash)
	if err := bc.verifyCommit(bc.passingHash, true); err != nil {
		return fmt.Errorf("verification of known-passing commit %s failed: %w", bc.passingHash, err)
	}
	fmt.Println("Verification succeeded: fuzzer passed on known-passing commit as expected.")

	// Run bisection
	fmt.Println("Both bounds verified. Commencing bisection...")
	if err := bc.performBisect(); err != nil {
		return fmt.Errorf("bisection failed: %w", err)
	}

	return nil
}

type syncBuildRunResult int

const (
	SyncBuildRunPassed syncBuildRunResult = iota
	SyncBuildRunFailed
	SyncBuildRunError // sync or build failed
)

// syncBuildRun runs gclient sync, builds the fuzzer, and runs the test case, returning the outcome.
// Note: is intentionally a receiver on taskConfig instead of bisectConfig, because bisectStep use/need the whole
//
//	bisectConfig state
func (t *taskConfig) syncBuildRun() (syncBuildRunResult, error) {
	fmt.Println("--> Running gclient sync...")
	if _, err := t.runCmd("gclient", "sync"); err != nil {
		return SyncBuildRunError, fmt.Errorf("gclient sync failed: %w", err)
	}

	fuzzerName := filepath.Base(t.fuzzer)
	fmt.Printf("--> Building fuzzer %s...\n", fuzzerName)
	if _, err := t.runCmd("autoninja", "-C", t.build, fuzzerName); err != nil {
		return SyncBuildRunError, fmt.Errorf("build failed: %w", err)
	}

	fmt.Printf("--> Running fuzzer %s against test case...\n", fuzzerName)
	_, err := t.runCmd(t.fuzzer, t.bisectFile)
	if err != nil {
		return SyncBuildRunFailed, nil
	}
	return SyncBuildRunPassed, nil
}

// verifyCommit checks out a commit, calls syncBuildRun, and asserts that it matches the expected outcome.
func (bc *bisectConfig) verifyCommit(hash string, expectPass bool) error {
	if _, err := bc.runCmd("git", "checkout", hash); err != nil {
		return fmt.Errorf("failed to checkout %s: %w", hash, err)
	}

	result, err := bc.syncBuildRun()
	if result == SyncBuildRunError {
		return fmt.Errorf("execution at %s failed: %w", hash, err)
	}

	passed := result == SyncBuildRunPassed
	if passed != expectPass {
		return fmt.Errorf("fuzzer execution returned pass=%v, but expected pass=%v", passed, expectPass)
	}
	return nil
}

// performBisect executes the standard git bisect routine by launching the current tool as an internal step subcommand.
func (bc *bisectConfig) performBisect() error {
	// Guarantee git bisect reset is called when bisection finishes
	defer func() {
		_, _ = bc.runCmd("git", "bisect", "reset")
	}()

	if _, err := bc.runCmd("git", "bisect", "start"); err != nil {
		return fmt.Errorf("failed to start git bisect: %w", err)
	}

	var err error
	if bc.isFix {
		// "bad" in git bisect is the target state we want to identify (the fix).
		// "good" in git bisect is the starting/original state (the breakage).
		_, err = bc.runCmd("git", "bisect", "bad", bc.passingHash)
		if err == nil {
			_, err = bc.runCmd("git", "bisect", "good", bc.failingHash)
		}
	} else {
		// "bad" in git bisect is the target state (the breakage).
		// "good" in git bisect is the working state (the passing).
		_, err = bc.runCmd("git", "bisect", "bad", bc.failingHash)
		if err == nil {
			_, err = bc.runCmd("git", "bisect", "good", bc.passingHash)
		}
	}
	if err != nil {
		return fmt.Errorf("failed to assign good/bad bounds to git bisect: %w", err)
	}

	self, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get absolute path of self: %w", err)
	}

	// Construct args to call ourselves back as the bisect-step handler
	args := []string{"bisect", "run", self, "-bisect-step", "-bisect=" + bc.bisectFile, "-build=" + bc.build}
	if bc.isFix {
		args = append(args, "-is-fix")
	}
	if bc.fuzzMode == FuzzModeIr {
		args = append(args, "-ir")
	}
	if bc.mesaMode {
		args = append(args, "-mesa")
	}
	if bc.verbose {
		args = append(args, "-verbose")
	}

	// Execute git bisect run
	if err := bc.runCmdUnbuffered("git", args...); err != nil {
		return fmt.Errorf("git bisect run failed: %w", err)
	}

	// Git bisect leaves the repo checked out at the identified commit.
	// Query HEAD to fetch the exact found commit hash.
	if foundHashBytes, err := bc.runCmd("git", "rev-parse", "HEAD"); err == nil {
		foundHash := strings.TrimSpace(string(foundHashBytes))
		bannerText := "FOUND BREAKAGE"
		if bc.isFix {
			bannerText = "FOUND FIX"
		}
		fmt.Println("\n================================================================================")
		fmt.Printf("%s @ %s\n", bannerText, foundHash)
		if logBytes, err := bc.runCmd("git", "log", "-1", "--oneline", foundHash); err == nil {
			fmt.Printf("%s", string(logBytes))
		}
		fmt.Println("================================================================================")
		fmt.Println()
	}

	return nil
}

// runBisectStep implements a single git bisect step invoked by git bisect run.
// It calls syncBuildRun and returns an appropriate exit codes (0 for good, 1 for bad, 125 for skipped/unbuildable).
func runBisectStep(t *taskConfig) error {
	fmt.Print("Running bisect step for commit: ")
	if out, err := t.runCmd("git", "rev-parse", "HEAD"); err == nil {
		fmt.Printf("%s", string(out))
	} else {
		fmt.Println("unknown")
	}

	result, err := t.syncBuildRun()
	if result == SyncBuildRunError {
		fmt.Printf("Step execution failed: %v. Skipping commit.\n", err)
		t.exitFn(125)
		return nil
	}

	// Translate to git bisect exit code
	if t.isFix {
		if result == SyncBuildRunPassed {
			fmt.Println("Fuzzer passed. This commit has the fix (bad for bisect).")
			t.exitFn(1)
		} else {
			fmt.Println("Fuzzer failed. This commit does NOT have the fix (good for bisect).")
			t.exitFn(0)
		}
	} else {
		if result == SyncBuildRunPassed {
			fmt.Println("Fuzzer passed. This commit does NOT have the breakage (good for bisect).")
			t.exitFn(0)
		} else {
			fmt.Println("Fuzzer failed. This commit has the breakage (bad for bisect).")
			t.exitFn(1)
		}
	}

	return nil
}

// resolveToHash converts a ref/hash/timestamp string to a complete git commit hash.
func resolveToHash(input string, execWrapper execwrapper.ExecWrapper) (string, error) {
	if input == "" {
		out, err := execWrapper.Command("git", "rev-parse", "HEAD").RunWithCombinedOutput()
		if err != nil {
			return "", err
		}
		return strings.TrimSpace(string(out)), nil
	}

	// Try treating it as a hash or ref first
	out, err := execWrapper.Command("git", "rev-parse", "--verify", input+"^{commit}").RunWithCombinedOutput()
	if err == nil {
		return strings.TrimSpace(string(out)), nil
	}

	// Try treating it as a date/time
	out, err = execWrapper.Command("git", "log", "-1", "--format=%H", "--before="+input).RunWithCombinedOutput()
	if err == nil {
		hash := strings.TrimSpace(string(out))
		if hash != "" {
			return hash, nil
		}
	}
	return "", fmt.Errorf("could not resolve '%s' to a commit hash", input)
}
