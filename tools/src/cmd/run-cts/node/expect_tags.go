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
	"encoding/json"
	"fmt"
	"os/exec"
	"regexp"
	"runtime"
	"slices"
	"strconv"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// runInfoObject is the JSON object that holds the run info.
type runInfoObject struct {
	RunInfo runInfo `json:"dawn-run-info"`
}

// runInfo is how the standalone runner was built, and the adapter that it
// selected.
type runInfo struct {
	Asan          bool        `json:"asan"`
	ClangCoverage bool        `json:"clangCoverage"`
	Adapter       adapterInfo `json:"adapter"`
}

// adapterInfo identifies a GPU adapter.
type adapterInfo struct {
	VendorID uint32 `json:"vendorID"`
	DeviceID uint32 `json:"deviceID"`
	Vendor   string `json:"vendor"`
	Device   string `json:"device"`
}

// worksOutTags returns true if the run works out its expectation tags.
func (c *cmd) worksOutTags() bool {
	return c.standalone && c.state.HasExpectationsFile()
}

// readRunInfo returns the run info that the standalone runner prints for
// --print-run-info.
func (c *cmd) readRunInfo() (*runInfo, error) {
	target, ctsArgs := c.target()
	output, err := target.stdout(append(c.runnerArgs(ctsArgs), "--print-run-info"))
	if err != nil {
		return nil, fmt.Errorf("%v could not print its run info: %w", runnerName, err)
	}
	return parseRunInfo(output)
}

// parseRunInfo returns the run info in output, which is what the runner wrote
// to stdout for --print-run-info. No output is an error that says no adapter
// was found, which is when the runner prints nothing.
func parseRunInfo(output []byte) (*runInfo, error) {
	if len(output) == 0 {
		return nil, fmt.Errorf("%v found no adapter to work out the expectation tags from (it printed no run info); check -backend and -adapter", runnerName)
	}
	parsed := runInfoObject{}
	if err := json.Unmarshal(output, &parsed); err != nil {
		return nil, fmt.Errorf("failed to parse the run info '%s' of %v: %w", output, runnerName, err)
	}
	return &parsed.RunInfo, nil
}

// addAutomaticTags adds the tags that describe the run to those of
// -expect-tags, and prints the tags of the run.
func (c *cmd) addAutomaticTags(fsReader oswrapper.FilesystemReader) error {
	info, err := c.readRunInfo()
	if err != nil {
		return err
	}
	automaticTags, err := c.automaticTags(fsReader, info)
	if err != nil {
		return err
	}
	runTags := c.state.AddAutomaticTags(automaticTags)
	fmt.Printf("Expectation tags: %v\n", result.TagsToString(runTags))
	return nil
}

// automaticTags returns the tags that Chromium's GPU test harness gives a run
// on the same OS, device, adapter and build with the same options, except for
// those of the driver version, the memory, the display server, the build type,
// ANGLE and the renderer.
func (c *cmd) automaticTags(fsReader oswrapper.FilesystemReader, info *runInfo) ([]string, error) {
	tags, err := c.platformTags(fsReader)
	if err != nil {
		return nil, err
	}
	tags = append(tags, gpuTags(info.Adapter)...)
	tags = append(tags, buildTags(info)...)
	return append(tags, c.optionTags()...), nil
}

// eitherTag returns tag if condition is true, and otherwise if not.
func eitherTag(condition bool, tag, otherwise string) string {
	if condition {
		return tag
	}
	return otherwise
}

// sanitizeTag returns tag in lower case with dashes for its spaces and
// underscores, as Chromium spells the tags that it makes from the names of
// things.
func sanitizeTag(tag string) string {
	return strings.NewReplacer(" ", "-", "_", "-").Replace(strings.ToLower(tag))
}

// buildTags returns the tags for how the runner was built.
func buildTags(info *runInfo) []string {
	return []string{
		eitherTag(info.Asan, "asan", "no-asan"),
		eitherTag(info.ClangCoverage, "clang-coverage", "no-clang-coverage"),
	}
}

// optionTags returns the tags for the options of the run: backend validation,
// the shader compiler on Windows, and those that no option of run-cts changes.
func (c *cmd) optionTags() []string {
	tags := []string{eitherTag(c.flags.validate, "dawn-backend-validation", "dawn-no-backend-validation")}
	if !c.flags.adb && runtime.GOOS == "windows" {
		tags = append(tags, eitherTag(c.flags.fxc, "webgpu-dxc-disabled", "webgpu-dxc-enabled"))
	}
	// run-cts allows unsafe APIs and never runs the CTS in a worker.
	return append(tags, "unsafe-webgpu-enabled", "webgpu-no-worker")
}

// platformTags returns the tags for the machine or device that the runner
// runs on.
func (c *cmd) platformTags(fsReader oswrapper.FilesystemReader) ([]string, error) {
	target, _ := c.target()
	if adb, ok := target.(*adbTarget); ok {
		properties, err := adb.androidProperties()
		if err != nil {
			return nil, err
		}
		return androidTags(properties), nil
	}
	return hostTags(fsReader), nil
}

// hostTags returns the tags for this machine: its OS and OS version,
// "desktop", its architecture and the bits of its CPU.
func hostTags(fsReader oswrapper.FilesystemReader) []string {
	tags := append(hostOSTags(fsReader), "desktop")
	switch runtime.GOARCH {
	case "amd64":
		tags = append(tags, "arch-x86_64")
	case "arm64":
		tags = append(tags, "arch-arm64")
	}
	// The runner is taken to be built for the architecture that run-cts is.
	return append(tags, fmt.Sprintf("target-cpu-%d", strconv.IntSize))
}

// hostOSTags returns the tag for the OS of this machine, and the tag for its
// version if Chromium has one.
func hostOSTags(fsReader oswrapper.FilesystemReader) []string {
	osTag, versionTag := "", ""
	switch runtime.GOOS {
	case "linux":
		osTag, versionTag = "linux", sanitizeTag(linuxDistribution(fsReader))
	case "windows":
		osTag, versionTag = "win", windowsVersionTag(windowsVerOutput())
	case "darwin":
		osTag, versionTag = "mac", macVersionTag(darwinVersion())
	default:
		return nil
	}
	if versionTag == "" {
		return []string{osTag}
	}
	return []string{osTag, versionTag}
}

// linuxDistribution returns the ID of os-release(5), such as "ubuntu", or ""
// if there is none. Chromium also falls back on lsb-release.
func linuxDistribution(fsReader oswrapper.FilesystemReader) string {
	for _, path := range []string{"/etc/os-release", "/usr/lib/os-release"} {
		content, err := fsReader.ReadFile(path)
		if err != nil {
			continue
		}
		for _, line := range strings.Split(string(content), "\n") {
			if id, ok := strings.CutPrefix(line, "ID="); ok {
				return strings.Trim(strings.TrimSpace(id), `"'`)
			}
		}
		return ""
	}
	return ""
}

// windowsVerOutput returns what `ver` prints on this machine, such as
// "Microsoft Windows [Version 10.0.22631.4460]", or "" if it fails.
func windowsVerOutput() string {
	output, err := exec.Command("cmd", "/c", "ver").Output()
	if err != nil {
		return ""
	}
	return string(output)
}

// windowsBuildNumber matches the build number of Windows 10 and 11 in the
// output of `ver`.
var windowsBuildNumber = regexp.MustCompile(`\b10\.0\.(\d+)`)

// windowsVersionTag returns "win10" or "win11" for the output of `ver`, or ""
// for another version of Windows.
func windowsVersionTag(verOutput string) string {
	match := windowsBuildNumber.FindStringSubmatch(verOutput)
	if match == nil {
		return ""
	}
	// Windows 11 kept the version 10.0 and starts at build 22000.
	if build, err := strconv.Atoi(match[1]); err == nil && build >= 22000 {
		return "win11"
	}
	return "win10"
}

// darwinVersion returns the major version of the Darwin kernel of this
// machine, or 0 if it cannot be read.
func darwinVersion() int {
	output, err := exec.Command("uname", "-r").Output()
	if err != nil {
		return 0
	}
	major, err := strconv.Atoi(strings.Split(strings.TrimSpace(string(output)), ".")[0])
	if err != nil {
		return 0
	}
	return major
}

// macVersionTag returns the name of the macOS version with the Darwin kernel
// darwinVersion, such as "sequoia" or "macos26", or "" if Chromium has no name
// for it.
func macVersionTag(darwinVersion int) string {
	switch {
	case darwinVersion < 21:
		return ""
	case darwinVersion == 21:
		return "monterey"
	case darwinVersion == 22:
		return "ventura"
	case darwinVersion == 23:
		return "sonoma"
	case darwinVersion == 24:
		return "sequoia"
	}
	// From Darwin 25 the macOS version is one more than the Darwin version.
	return fmt.Sprintf("macos%d", darwinVersion+1)
}

// androidProperties are the system properties of an Android device that its
// tags come from.
type androidProperties struct {
	release string // ro.build.version.release, such as "13"
	buildID string // ro.build.id, such as "TQ3A.230805.001"
	model   string // ro.product.model, such as "Pixel 6"
	abi     string // ro.product.cpu.abi, such as "arm64-v8a"
}

// androidProperties reads the system properties of the device.
func (t *adbTarget) androidProperties() (androidProperties, error) {
	output, err := t.shell("getprop ro.build.version.release; getprop ro.build.id; getprop ro.product.model; getprop ro.product.cpu.abi")
	if err != nil {
		return androidProperties{}, err
	}
	values := strings.Split(strings.ReplaceAll(output, "\r", ""), "\n")
	if len(values) < 4 {
		return androidProperties{}, fmt.Errorf("failed to read the properties of the device: getprop printed '%v'", output)
	}
	return androidProperties{release: values[0], buildID: values[1], model: values[2], abi: values[3]}, nil
}

// androidTags returns the tags for a device with properties: "android", its
// Android release, "mobile", its model and the bits of its CPU. Android
// desktop devices are not told apart.
func androidTags(properties androidProperties) []string {
	tags := append([]string{"android"}, androidReleaseTags(properties.release, properties.buildID)...)
	tags = append(tags, "mobile", sanitizeTag("android-"+properties.model))
	return append(tags, eitherTag(strings.Contains(properties.abi, "64"), "target-cpu-64", "target-cpu-32"))
}

// androidReleaseNames maps the first letter of the build ID of an Android
// release up to 13 to the name that Chromium uses instead of the letter.
var androidReleaseNames = map[string]string{
	"k": "kitkat",
	"l": "lollipop",
	"m": "marshmallow",
	"n": "nougat",
	"o": "oreo",
	"p": "pie",
}

// androidReleaseTags returns the tags for the Android release, such as "13",
// of a build with buildID: its number and, up to Android 13, its letter or
// name.
func androidReleaseTags(release, buildID string) []string {
	// The release of a preview build is a codename, which has no tag.
	major, err := strconv.Atoi(strings.Split(release, ".")[0])
	if err != nil {
		return nil
	}
	tags := []string{fmt.Sprintf("android-%d", major)}
	// Up to Android 13 the build ID starts with the letter of the release.
	if major <= 13 && buildID != "" {
		letter := strings.ToLower(buildID[:1])
		if name, ok := androidReleaseNames[letter]; ok {
			letter = name
		}
		tags = append(tags, "android-"+letter)
	}
	return tags
}

// SwiftShader's vendor and device ID.
const (
	swiftShaderVendorID = 0x1ae0
	swiftShaderDeviceID = 0xc0de
)

// vendorTagOfID maps a GPU vendor ID to the tag that Chromium's expectations
// have for the vendor.
var vendorTagOfID = map[uint32]string{
	0x1002:              "amd",
	0x1010:              "imagination",
	0x106b:              "apple",
	0x10de:              "nvidia",
	0x13b5:              "arm",
	0x1414:              "microsoft",
	0x144d:              "samsung-electronics-co.-ltd.",
	swiftShaderVendorID: "google",
	0x4d4f4351:          "qualcomm",
	0x5143:              "qualcomm",
	0x8086:              "intel",
}

// Intel's GPU generation is the high byte of the device ID.
const intelGenerationMask = 0xff00

var (
	intelGen9  = []uint32{0x1900, 0x3100, 0x3e00, 0x5900, 0x5a00, 0x9b00}
	intelGen12 = []uint32{0x4c00, 0x9a00, 0x4900, 0x4600, 0x4f00, 0x5600, 0xa700, 0x7d00}
)

// gpuTags returns the tags for adapter: its vendor, its vendor and device,
// the generation of an Intel GPU, and whether it is SwiftShader.
func gpuTags(adapter adapterInfo) []string {
	vendor := vendorTag(adapter)
	// An adapter without a device ID is known by its name, as Apple's are.
	device := fmt.Sprintf("%v-0x%x", vendor, adapter.DeviceID)
	if adapter.DeviceID == 0 {
		device = sanitizeTag(vendor + "-" + adapter.Device)
	}
	tags := []string{vendor, device}
	if vendor == "intel" {
		tags = append(tags, intelGenerationTags(adapter.DeviceID)...)
	}
	isSwiftShader := adapter.VendorID == swiftShaderVendorID && adapter.DeviceID == swiftShaderDeviceID
	return append(tags, eitherTag(isSwiftShader, "webgpu-adapter-swiftshader", "webgpu-adapter-default"))
}

// vendorTag returns the tag for the vendor of adapter: that of its vendor ID
// or, for an ID without one, the first word of the vendor's name.
func vendorTag(adapter adapterInfo) string {
	if vendor, ok := vendorTagOfID[adapter.VendorID]; ok {
		return vendor
	}
	if words := strings.Fields(adapter.Vendor); len(words) > 0 {
		return sanitizeTag(words[0])
	}
	return "unknown-gpu"
}

// intelGenerationTags returns the tag for the generation of the Intel GPU with
// deviceID, if Chromium has one.
func intelGenerationTags(deviceID uint32) []string {
	generation := deviceID & intelGenerationMask
	if slices.Contains(intelGen9, generation) {
		return []string{"intel-gen-9"}
	}
	if slices.Contains(intelGen12, generation) {
		return []string{"intel-gen-12"}
	}
	return nil
}
