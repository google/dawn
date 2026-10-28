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

package node

import (
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"github.com/stretchr/testify/require"
)

// fakeAdbScript runs adb commands against a "device" that is a directory on
// this machine. The shell command runs as a child of the script rather than
// replacing it, so that killing the adb client leaves the device process
// running, as with a real device. Setting FAKE_ADB_ERROR makes shell commands
// fail with that error, as when no device is connected.
const fakeAdbScript = `#!/bin/sh
if [ "$1" = "-s" ]; then
  shift 2
fi
command=$1
shift
case $command in
shell)
  if [ -n "$FAKE_ADB_ERROR" ]; then
    echo "adb: $FAKE_ADB_ERROR" >&2
    exit 1
  fi
  sh -c "$*"
  exit $?
  ;;
push)
  mkdir -p "$(dirname "$2")" && cp -R "$1" "$2"
  ;;
*)
  echo "fake adb: unsupported command $command" >&2
  exit 1
  ;;
esac
`

// installFakeAdb puts a fake adb first on PATH.
func installFakeAdb(t *testing.T) {
	t.Helper()
	if runtime.GOOS == "windows" {
		t.Skip("the fake adb is a shell script")
	}
	dir := t.TempDir()
	require.NoError(t, os.WriteFile(filepath.Join(dir, "adb"), []byte(fakeAdbScript), 0777))
	t.Setenv("PATH", dir+string(os.PathListSeparator)+os.Getenv("PATH"))
	// Processes started on the "device" act as the runner.
	t.Setenv(fakeRunnerEnv, "1")
}

// newAdbTestRunner returns a cmdlineRunner that runs cases with the fake runner
// on a fake device whose directory already holds the runner and the CTS.
func newAdbTestRunner(t *testing.T, cases ...string) *cmdlineRunner {
	t.Helper()
	runner, ctsDir := newTestRunner(t, cases...)
	adb, err := exec.LookPath("adb")
	require.NoError(t, err)
	target := &adbTarget{adb: adb, serial: "fake-serial", deviceDir: t.TempDir()}
	exe, err := os.Executable()
	require.NoError(t, err)
	require.NoError(t, os.MkdirAll(filepath.Dir(target.devicePath(deviceRunner)), 0777))
	require.NoError(t, os.Symlink(exe, target.devicePath(deviceRunner)))
	require.NoError(t, os.Symlink(ctsDir, target.devicePath(deviceCTS)))
	runner.target = target
	runner.args = []string{"--cts-dir", target.devicePath(deviceCTS)}
	return runner
}

// A crash on the device is reported with the signal that adb shell relays.
func TestAdbCrashWithoutRecovery(t *testing.T) {
	installFakeAdb(t)
	runner := newAdbTestRunner(t, "suite:a:", "suite:crash:", "suite:c:")
	runner.recover = false
	results := runCmdline(t, runner)
	require.Equal(t, common.Pass, results["suite:a:"].Status)
	crashed := results["suite:crash:"]
	require.Equal(t, common.Crash, crashed.Status)
	require.Contains(t, crashed.Message, "[ERROR] crashing in suite:crash:\n")
	require.Contains(t, crashed.Message, "exit status 137 (signal 9 on the device: killed)")
	require.Contains(t, results["suite:c:"].Message, "not run")
}

// A hang on the device is ended by killing the runner there, not just the adb
// client, and the run continues.
func TestAdbTimeout(t *testing.T) {
	installFakeAdb(t)
	runner := newAdbTestRunner(t, "suite:a:", "suite:hang:", "suite:c:")
	runner.caseTimeout = 500 * time.Millisecond
	start := time.Now()
	results := runCmdline(t, runner)
	require.Less(t, time.Since(start), 10*time.Second)
	require.Equal(t, common.Pass, results["suite:a:"].Status)
	require.Equal(t, common.Timeout, results["suite:hang:"].Status)
	require.Equal(t, common.Pass, results["suite:c:"].Status)
}

// When the adb client fails before the runner prints its PID, the batch is
// reported as not run with the client's error.
func TestAdbClientFailure(t *testing.T) {
	installFakeAdb(t)
	t.Setenv("FAKE_ADB_ERROR", "no devices/emulators found")
	runner := newAdbTestRunner(t, "suite:a:", "suite:b:")
	results := runCmdline(t, runner)
	for _, res := range results {
		require.Equal(t, common.Fail, res.Status)
		require.Contains(t, res.Message, "not run: could not start the runner: adb shell did not start the runner: exit status 1\nadb: no devices/emulators found\n")
	}
}
