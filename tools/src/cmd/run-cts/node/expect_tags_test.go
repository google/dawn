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
	"testing"

	"github.com/stretchr/testify/require"
)

// No output from the runner is an error that says no adapter was found, and
// output that is not the JSON object is an error that quotes it.
func TestParseRunInfo(t *testing.T) {
	info, err := parseRunInfo([]byte(`{
  "dawn-run-info": {
    "asan": true,
    "clangCoverage": false,
    "adapter": {
      "vendorID": 32902,
      "deviceID": 39877,
      "vendor": "intel",
      "device": "UHD Graphics"
    }
  }
}
`))
	require.NoError(t, err)
	require.Equal(t, &runInfo{Asan: true, Adapter: adapterInfo{VendorID: 0x8086, DeviceID: 0x9bc5, Vendor: "intel", Device: "UHD Graphics"}}, info)

	_, err = parseRunInfo(nil)
	require.ErrorContains(t, err, "found no adapter to work out the expectation tags from (it printed no run info)")

	_, err = parseRunInfo([]byte(`{ "dawn-run-info":`))
	require.ErrorContains(t, err, `failed to parse the run info '{ "dawn-run-info":'`)
}

// An adapter gets a tag for its vendor, from its vendor ID or else its vendor
// name, one for its device, from its device ID or else its name, one for the
// generation of an Intel GPU, and one that tells SwiftShader from the others.
func TestGPUTags(t *testing.T) {
	for _, test := range []struct {
		adapter adapterInfo
		tags    []string
	}{
		{adapterInfo{VendorID: 0x8086, DeviceID: 0x9bc5, Vendor: "intel"}, []string{"intel", "intel-0x9bc5", "intel-gen-9", "webgpu-adapter-default"}},
		{adapterInfo{VendorID: 0x8086, DeviceID: 0x9a49, Vendor: "intel"}, []string{"intel", "intel-0x9a49", "intel-gen-12", "webgpu-adapter-default"}},
		{adapterInfo{VendorID: 0x8086, DeviceID: 0x0412, Vendor: "intel"}, []string{"intel", "intel-0x412", "webgpu-adapter-default"}},
		{adapterInfo{VendorID: 0x106b, Vendor: "apple", Device: "Apple M2"}, []string{"apple", "apple-apple-m2", "webgpu-adapter-default"}},
		{adapterInfo{VendorID: 0x1234, DeviceID: 0x1, Vendor: "Acme Graphics Inc."}, []string{"acme", "acme-0x1", "webgpu-adapter-default"}},
		{adapterInfo{VendorID: 0x1234, DeviceID: 0x1}, []string{"unknown-gpu", "unknown-gpu-0x1", "webgpu-adapter-default"}},
		{adapterInfo{VendorID: 0x1ae0, DeviceID: 0xc0de, Vendor: "google"}, []string{"google", "google-0xc0de", "webgpu-adapter-swiftshader"}},
	} {
		require.Equal(t, test.tags, gpuTags(test.adapter), "%+v", test.adapter)
	}
}

// An Android device gets tags for the number of its release and, up to
// Android 13, for the release's letter or name, for its model and for the
// bits of its ABI.
func TestAndroidTags(t *testing.T) {
	for _, test := range []struct {
		properties androidProperties
		tags       []string
	}{
		{androidProperties{"13", "TQ3A.230805.001", "Pixel 6", "arm64-v8a"}, []string{"android", "android-13", "android-t", "mobile", "android-pixel-6", "target-cpu-64"}},
		{androidProperties{"14", "UQ1A.240205.002", "SM-S926B", "arm64-v8a"}, []string{"android", "android-14", "mobile", "android-sm-s926b", "target-cpu-64"}},
		{androidProperties{"9", "PQ3A.190801.002", "Pixel 2", "armeabi-v7a"}, []string{"android", "android-9", "android-pie", "mobile", "android-pixel-2", "target-cpu-32"}},
		{androidProperties{"Baklava", "BP21.241121.009", "Pixel 9", "arm64-v8a"}, []string{"android", "mobile", "android-pixel-9", "target-cpu-64"}},
	} {
		require.Equal(t, test.tags, androidTags(test.properties), "%+v", test.properties)
	}
}

// Windows 11 is told from Windows 10 by its build number, and other versions
// of Windows have no tag.
func TestWindowsVersionTag(t *testing.T) {
	for verOutput, tag := range map[string]string{
		"\r\nMicrosoft Windows [Version 10.0.19045.5131]\r\n": "win10",
		"\r\nMicrosoft Windows [Version 10.0.22631.4460]\r\n": "win11",
		"\r\nMicrosoft Windows [Version 6.3.9600]\r\n":        "",
		"": "",
	} {
		require.Equal(t, tag, windowsVersionTag(verOutput), verOutput)
	}
}

// macOS is named up to Sequoia and numbered from macOS 26, and has no tag
// before Monterey.
func TestMacVersionTag(t *testing.T) {
	for darwinVersion, tag := range map[int]string{
		0:  "",
		20: "",
		21: "monterey",
		23: "sonoma",
		24: "sequoia",
		25: "macos26",
	} {
		require.Equal(t, tag, macVersionTag(darwinVersion), darwinVersion)
	}
}
