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
	"archive/zip"
	"compress/flate"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
	"time"
)

// Values of -stage.
const (
	stageAuto   = "auto"   // copy the parts whose hash differs from the device's
	stageAlways = "always" // copy every part
	stageNever  = "never"  // use what is on the device
)

// More paths on the device, relative to the device directory.
const (
	deviceStamp      = "stamp"
	deviceArchive    = "cts.zip"
	deviceRunnerTemp = deviceRunner + ".new"
)

// cmdlineMarker is the file that the runner and cmdline.js look for to find
// the root of the CTS.
const cmdlineMarker = "src/common/runtime/cmdline.ts"

// stagedPart is one part of what the device needs.
type stagedPart struct {
	name  string // name of the part in the stamp file
	hash  string // SHA-256 of the part's file names and contents
	stale bool   // the device does not have this content
}

// newStagedPart hashes the files of a part, which are relative to dir, and
// marks the part stale unless deviceHashes has that hash for it.
func newStagedPart(name, dir string, files []string, deviceHashes map[string]string) (*stagedPart, error) {
	hash, err := hashFiles(dir, files)
	if err != nil {
		return nil, err
	}
	return &stagedPart{name: name, hash: hash, stale: deviceHashes[name] != hash}, nil
}

// stage copies the runner and the CTS onto the device. The device directory
// holds a stamp file with the hash of each part it has, and unless force is
// set, a part whose hash matches is not copied again.
func (t *adbTarget) stage(runner, ctsDir string, force bool) error {
	start := time.Now()
	deviceHashes := map[string]string{}
	if !force {
		var err error
		if deviceHashes, err = t.deviceHashes(); err != nil {
			return err
		}
	}
	runnerPart, err := newStagedPart("runner", filepath.Dir(runner), []string{filepath.Base(runner)}, deviceHashes)
	if err != nil {
		return err
	}
	files, err := ctsFiles(ctsDir)
	if err != nil {
		return err
	}
	ctsPart, err := newStagedPart("cts", ctsDir, files, deviceHashes)
	if err != nil {
		return err
	}
	if !runnerPart.stale && !ctsPart.stale {
		fmt.Println("stage: the device is up to date")
		return nil
	}
	if runnerPart.stale {
		if err := t.push(runner, t.devicePath(deviceRunnerTemp)); err != nil {
			return err
		}
	}
	if ctsPart.stale {
		if err := t.pushCTS(ctsDir, files); err != nil {
			return err
		}
	}
	if err := t.install(runnerPart, ctsPart); err != nil {
		return err
	}
	staged := []string{}
	if runnerPart.stale {
		staged = append(staged, "the runner")
	}
	if ctsPart.stale {
		staged = append(staged, "the CTS")
	}
	fmt.Printf("stage: copied %v onto the device in %.1fs\n", strings.Join(staged, " and "), time.Since(start).Seconds())
	return nil
}

// deviceHashes returns the hash of each part the device has, read from its
// stamp file of "<name> <hash>" lines. A device without a stamp has nothing.
func (t *adbTarget) deviceHashes() (map[string]string, error) {
	stamp, err := t.shell("cat " + shellQuote(t.devicePath(deviceStamp)) + " 2>/dev/null; true")
	if err != nil {
		return nil, err
	}
	hashes := map[string]string{}
	for _, line := range strings.Split(stamp, "\n") {
		if name, hash, found := strings.Cut(strings.TrimSpace(line), " "); found {
			hashes[name] = hash
		}
	}
	return hashes, nil
}

// push copies a host file to the device.
func (t *adbTarget) push(local, remote string) error {
	out, err := t.command("push", local, remote).CombinedOutput()
	if err != nil {
		return fmt.Errorf("adb push failed: %w\n%s", err, out)
	}
	return nil
}

// pushCTS packs files, which are relative to ctsDir, into a zip and copies it
// to the device.
func (t *adbTarget) pushCTS(ctsDir string, files []string) error {
	tempDir, err := os.MkdirTemp("", "run-cts-stage")
	if err != nil {
		return err
	}
	defer os.RemoveAll(tempDir)
	archive := filepath.Join(tempDir, deviceArchive)
	if err := writeArchive(archive, ctsDir, files); err != nil {
		return err
	}
	return t.push(archive, t.devicePath(deviceArchive))
}

// install moves the pushed parts into place on the device. It first rewrites
// the stamp without the stale parts and adds each one back once it is in
// place, so that an interrupted install never leaves a stamp that matches.
func (t *adbTarget) install(parts ...*stagedPart) error {
	upToDate := ""
	for _, part := range parts {
		if !part.stale {
			upToDate += part.name + " " + part.hash + "\n"
		}
	}
	commands := []string{
		"cd " + shellQuote(t.deviceDir),
		"printf %s " + shellQuote(upToDate) + " > " + deviceStamp,
	}
	for _, part := range parts {
		if !part.stale {
			continue
		}
		switch part.name {
		case "runner":
			commands = append(commands,
				"mv "+deviceRunnerTemp+" "+deviceRunner,
				"chmod 755 "+deviceRunner)
		case "cts":
			commands = append(commands,
				"rm -rf "+deviceCTS,
				"unzip -q "+deviceArchive+" -d "+deviceCTS,
				"rm "+deviceArchive)
		}
		commands = append(commands, "echo "+part.name+" "+part.hash+" >> "+deviceStamp)
	}
	_, err := t.shell(strings.Join(commands, " && "))
	return err
}

// ctsFiles returns the paths of the CTS files to stage, relative to ctsDir and
// with forward slashes, in lexical order. The *.map source maps are left out:
// nothing reads them on the device.
func ctsFiles(ctsDir string) ([]string, error) {
	files := []string{}
	err := filepath.WalkDir(filepath.Join(ctsDir, "out-node"), func(p string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if d.IsDir() || strings.HasSuffix(p, ".map") {
			return nil
		}
		rel, err := filepath.Rel(ctsDir, p)
		if err != nil {
			return err
		}
		files = append(files, filepath.ToSlash(rel))
		return nil
	})
	if err != nil {
		return nil, err
	}
	return append(files, cmdlineMarker), nil
}

// hashFiles returns the SHA-256 of the names, sizes and contents of files,
// which are relative to dir.
func hashFiles(dir string, files []string) (string, error) {
	h := sha256.New()
	for _, file := range files {
		f, err := os.Open(filepath.Join(dir, filepath.FromSlash(file)))
		if err != nil {
			return "", err
		}
		info, err := f.Stat()
		if err == nil {
			fmt.Fprintf(h, "%v\x00%v\x00", file, info.Size())
			_, err = io.Copy(h, f)
		}
		f.Close()
		if err != nil {
			return "", err
		}
	}
	return hex.EncodeToString(h.Sum(nil)), nil
}

// writeArchive writes files, which are relative to dir, to a zip whose entry
// names are those relative paths, so that the archive mirrors the CTS
// directory. It has no directory entries, since unzip creates the missing
// directories.
func writeArchive(zipPath, dir string, files []string) error {
	f, err := os.Create(zipPath)
	if err != nil {
		return err
	}
	defer f.Close()
	zw := zip.NewWriter(f)
	zw.RegisterCompressor(zip.Deflate, fastDeflate)
	for _, file := range files {
		if err := addFileToArchive(zw, dir, file); err != nil {
			return err
		}
	}
	return zw.Close()
}

// fastDeflate returns a deflate writer that favours speed over size:
// compressing harder takes longer than pushing the extra megabytes.
func fastDeflate(out io.Writer) (io.WriteCloser, error) {
	return flate.NewWriter(out, flate.BestSpeed)
}

// addFileToArchive writes file, which is relative to dir, to zw. The entries
// carry a fixed date, so that one CTS build always packs to the same bytes.
func addFileToArchive(zw *zip.Writer, dir, file string) error {
	f, err := os.Open(filepath.Join(dir, filepath.FromSlash(file)))
	if err != nil {
		return err
	}
	defer f.Close()
	header := &zip.FileHeader{
		Name:     file,
		Method:   zip.Deflate,
		Modified: time.Date(1980, 1, 1, 0, 0, 0, 0, time.UTC),
	}
	header.SetMode(0644)
	w, err := zw.CreateHeader(header)
	if err != nil {
		return fmt.Errorf("cannot add '%v' to the archive: %w", file, err)
	}
	_, err = io.Copy(w, f)
	return err
}
