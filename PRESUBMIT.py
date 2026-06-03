# Copyright 2022 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import hashlib
import re
import sys

PRESUBMIT_VERSION = '2.0.0'

NONINCLUSIVE_LANGUAGE_REGEXES = [
    re.compile(reg) for reg in [
        r"(?i)black[-_]?list",
        r"(?i)white[-_]?list",
        r"(?i)gr[ea]y[-_]?list",
        r"(?i)(first class citizen)",
        r"(?i)black[-_]?hat",
        r"(?i)white[-_]?hat",
        r"(?i)gr[ea]y[-_]?hat",
        r"(?i)master",
        r"(?i)slave",
        r"(?i)\bhim\b",
        r"(?i)\bhis\b",
        r"(?i)\bshe\b",
        r"(?i)\bher\b",
        r"(?i)\bguys\b",
        r"(?i)\bhers\b",
        r"(?i)\bman\b",
        r"(?i)\bwoman\b",
        r"(?i)\she\s",
        r"(?i)\she$",
        r"(?i)^he\s",
        r"(?i)^he$",
        r"(?i)\she['|\u2019]d\s",
        r"(?i)\she['|\u2019]d$",
        r"(?i)^he['|\u2019]d\s",
        r"(?i)^he['|\u2019]d$",
        r"(?i)\she['|\u2019]s\s",
        r"(?i)\she['|\u2019]s$",
        r"(?i)^he['|\u2019]s\s",
        r"(?i)^he['|\u2019]s$",
        r"(?i)\she['|\u2019]ll\s",
        r"(?i)\she['|\u2019]ll$",
        r"(?i)^he['|\u2019]ll\s",
        r"(?i)^he['|\u2019]ll$",
        r"(?i)grandfather",
        r"(?i)\bmitm\b",
        r"(?i)\bcrazy\b",
        r"(?i)\binsane\b",
        r"(?i)\bblind\sto\b",
        r"(?i)\bflying\sblind\b",
        r"(?i)\bblind\seye\b",
        r"(?i)\bcripple\b",
        r"(?i)\bcrippled\b",
        r"(?i)\bdumb\b",
        r"(?i)\bdummy\b",
        r"(?i)\bparanoid\b",
        r"(?i)\bsane\b",
        r"(?i)\bsanity\b",
        r"(?i)red[-_]?line",
    ]
]

LINT_FILTERS = []

EXPECTED_LICENSE_TEXT = {
    "//": [
        "//",
        "// Redistribution and use in source and binary forms, with or without",
        "// modification, are permitted provided that the following conditions are met:",
        "//",
        "// 1. Redistributions of source code must retain the above copyright notice, this",
        "//    list of conditions and the following disclaimer.",
        "//",
        "// 2. Redistributions in binary form must reproduce the above copyright notice,",
        "//    this list of conditions and the following disclaimer in the documentation",
        "//    and/or other materials provided with the distribution.",
        "//",
        "// 3. Neither the name of the copyright holder nor the names of its",
        "//    contributors may be used to endorse or promote products derived from",
        "//    this software without specific prior written permission.",
        "//",
        "// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"",
        "// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
        "// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE",
        "// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE",
        "// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
        "// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR",
        "// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER",
        "// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,",
        "// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE",
        "// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
    ],
    "#": [
        "#",
        "# Redistribution and use in source and binary forms, with or without",
        "# modification, are permitted provided that the following conditions are met:",
        "#",
        "# 1. Redistributions of source code must retain the above copyright notice, this",
        "#    list of conditions and the following disclaimer.",
        "#",
        "# 2. Redistributions in binary form must reproduce the above copyright notice,",
        "#    this list of conditions and the following disclaimer in the documentation",
        "#    and/or other materials provided with the distribution.",
        "#",
        "# 3. Neither the name of the copyright holder nor the names of its",
        "#    contributors may be used to endorse or promote products derived from",
        "#    this software without specific prior written permission.",
        "#",
        "# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"",
        "# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
        "# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE",
        "# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE",
        "# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
        "# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR",
        "# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER",
        "# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,",
        "# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE",
        "# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
    ]
}

# 'go fmt' has slightly different spacing rules relative to our C++
# style, so .go files need their own variant
EXPECTED_LICENSE_TEXT_GO = [
    "//",
    "// Redistribution and use in source and binary forms, with or without",
    "// modification, are permitted provided that the following conditions are met:",
    "//",
    "//  1. Redistributions of source code must retain the above copyright notice, this",
    "//     list of conditions and the following disclaimer.",
    "//",
    "//  2. Redistributions in binary form must reproduce the above copyright notice,",
    "//     this list of conditions and the following disclaimer in the documentation",
    "//     and/or other materials provided with the distribution.",
    "//",
    "//  3. Neither the name of the copyright holder nor the names of its",
    "//     contributors may be used to endorse or promote products derived from",
    "//     this software without specific prior written permission.",
    "//",
    "// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"",
    "// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
    "// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE",
    "// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE",
    "// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
    "// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR",
    "// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER",
    "// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,",
    "// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE",
    "// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
]


def _NonInclusiveFileFilter(file):
    """Filters files that are exempt from the non-inclusive language check."""
    filter_list = [
        "Doxyfile",  # References to main pages
        "PRESUBMIT.py",  # Non-inclusive language check data
        "PRESUBMIT.py.tint",  # Non-inclusive language check data
        "docs/dawn/debug_markers.md",  # External URL
        "docs/dawn/infra.md",  # Infra settings
        "docs/tint/spirv-input-output-variables.md",  # External URL
        "infra/config/global/generated/cr-buildbucket.cfg",  # Infra settings
        "infra/config/global/main.star",  # Infra settings
        "infra/kokoro/windows/build.bat",  # External URL
        "src/dawn/common/GPUInfo.cpp",  # External URL
        "src/dawn/common/ThreadLocal.cpp",  # External URL
        "src/dawn/native/CommandEncoder.cpp",  # External URL
        "src/dawn/native/metal/BackendMTL.mm",  # OSX Constant
        "src/dawn/native/metal/PhysicalDeviceMTL.mm",  # OSX deprecated API
        "src/dawn/native/vulkan/SamplerVk.cpp",  # External URL
        "src/dawn/native/vulkan/TextureVk.cpp",  # External URL
        "src/tools/src/cmd/run-cts/main.go",  # Terminal type name
        "src/dawn/samples/ComputeBoids.cpp",  # External URL
        "src/dawn/tests/end2end/DepthBiasTests.cpp",  # External URL
        "src/tint/transform/canonicalize_entry_point_io.cc",  # External URL
        "test/tint/samples/compute_boids.wgsl",  # External URL
        "third_party/directx-shader-compiler/BUILD.gn",  # Third party file
        "third_party/directx-shader-compiler/LICENSE",  # Third party file
        "third_party/directx-shader-compiler/build/message_compiler.py",  # Third party file
        "third_party/EGL-Registry/src/api/KHR/khrplatform.h",  # Third party file
        "tools/roll-all",  # Branch name
        "tools/src/container/key.go",  # External URL
        "go.sum",  # External URL
    ]
    return file.LocalPath().replace('\\', '/') not in filter_list


def CheckNonInclusiveLanguage(input_api, output_api):
    """Checks the files for non-inclusive language."""
    matches = []
    for f in input_api.AffectedFiles(include_deletes=False,
                                     file_filter=_NonInclusiveFileFilter):
        for line_num, line in enumerate(f.NewContents(), start=1):
            for reg in NONINCLUSIVE_LANGUAGE_REGEXES:
                if match := reg.search(line):
                    matches.append(
                        f"{f.LocalPath()} ({line_num}): found non-inclusive language: {match.group(0)}"
                    )

    if matches:
        return [
            output_api.PresubmitPromptWarning("Non-inclusive language found:",
                                              items=matches)
        ]

    return []


def _CalculateEnumeratedEntriesAndTypes(lines):
    """Returns a dictionary of enumerated entries, and a list of all the 'types' encountered.

    The implemented parsing is unsophisticated, and assumes a readable/well-formed .proto file.
    Things like unmatched '{}'s will cause a crash. Missing ';'s or writing something like `} message Foo {` will also
    cause misbehaviour.
    Constructs like this are normally bad style, so if really needed, adding support for them is left as an exercise for
    the reader.
    """
    push_re = re.compile(r'(\w+) {(.*)')
    value_re = re.compile(r'(\w+) = (\d+);(.*)')
    reserved_re = re.compile(r'^\s*reserved\s+(.*);(.*)')
    number_re = re.compile(r'\d+')
    pop_re = re.compile(r'}(.*)')

    prefix_stack = []
    prefix_str = ""
    enumerated_entries = {}
    oneof_scopes = set()
    types = []
    for l in lines:
        l = l.strip().rstrip()
        l = l.split("//", 1)[0]
        while l:
            if match := re.search(push_re, l):
                if l[:match.start()].strip() == "oneof":
                    oneof_scopes.add('.'.join(prefix_stack + [match.group(1)]))
                prefix_stack.append(match.group(1))
                prefix_str = '.'.join(prefix_stack)
                types.append(prefix_str)
                l = match.group(2)
                continue
            if match := re.search(reserved_re, l):
                new_numbers = number_re.findall(match.group(0))
                reserved_numbers = enumerated_entries.get(
                    f"{prefix_str}.reserved", [])
                reserved_numbers.extend(new_numbers)
                enumerated_entries[f"{prefix_str}.reserved"] = reserved_numbers
                l = match.group(1)
                continue
            if match := re.search(value_re, l):
                enumerated_entries[
                    f"{prefix_str}.{match.group(1)}"] = match.group(2)
                l = match.group(2)
                continue
            if match := re.search(pop_re, l):
                prefix_stack.pop()
                prefix_str = '.'.join(prefix_stack)
                l = match.group(1)
                continue
            l = ""

    return enumerated_entries, types, oneof_scopes


def CheckIRBinaryCompatibility(input_api, output_api):
    """Checks for changes to ir.proto that may cause compatibility issues"""
    proto_file = None
    old_entries, old_types = {}, []
    new_entries, new_types = {}, []
    for file in input_api.AffectedFiles(
            include_deletes=False,
            file_filter=lambda f: f.LocalPath().replace(
                '\\', '/') == "src/tint/utils/protos/ir/ir.proto"):
        if proto_file:
            return [
                output_api.PresubmitError(
                    f"Unexpectedly found more than one ir.proto in change, [{file.AbsoluteLocalPath()}, {proto_file}]"
                )
            ]
        proto_file = file.AbsoluteLocalPath()
        old_entries, old_types, old_oneofs = _CalculateEnumeratedEntriesAndTypes(
            file.OldContents())
        new_entries, new_types, new_oneofs = _CalculateEnumeratedEntriesAndTypes(
            file.NewContents())

    changes = []

    for k in old_entries:
        if k not in new_entries:
            entry_prefix = k.rsplit('.', 1)[0]
            reserved = new_entries.get(f"{entry_prefix}.reserved", [])
            if old_entries[k] in reserved:
                continue

            if entry_prefix in old_oneofs:
                parent_prefix = entry_prefix.rsplit('.', 1)[0]
                reserved = new_entries.get(f"{parent_prefix}.reserved", [])
                if old_entries[k] in reserved:
                    continue

            changes.append(
                f"entry '{k}' has been removed without reserving, old={old_entries[k]}"
            )
            continue

        if old_entries[k] != new_entries[k]:
            changes.append(
                f"entry '{k}' has changed, old={old_entries[k]}, new={new_entries[k]}"
            )
            continue

    for s in old_types:
        if s not in new_types:
            changes.append(f"type '{s}' has been removed")

    if changes:
        return [
            output_api.PresubmitError(
                "Incompatible changes detected in ir.proto", items=changes)
        ]
    return []


def CheckNoStaleGen(input_api, output_api):
    """Checks that Tint generated files are not stale."""
    sys.path += [input_api.change.RepositoryRoot()]

    import go_presubmit_support

    results = []
    try:
        go = go_presubmit_support.go_path(input_api)
        input_api.subprocess.check_call_out(
            [go, "run", "tools/src/cmd/gen/main.go", "--check-stale"],
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.change.RepositoryRoot())
    except input_api.subprocess.CalledProcessError as e:
        if input_api.is_committing:
            results.append(output_api.PresubmitError('%s' % (e, )))
        else:
            results.append(output_api.PresubmitPromptWarning('%s' % (e, )))
    return results


def CheckWebgpuHeaderDiff(input_api, output_api):
    """Checks that generated WebGPU C Headers are not stale."""
    results = []
    try:
        input_api.subprocess.check_call_out(
            [sys.executable, "third_party/webgpu-headers/cli", "check"],
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.change.RepositoryRoot())
    except input_api.subprocess.CalledProcessError as e:
        if input_api.is_committing:
            results.append(output_api.PresubmitError('%s' % (e, )))
        else:
            results.append(output_api.PresubmitPromptWarning('%s' % (e, )))
    return results


def _HasNoStrayWhitespaceFilter(file):
    """Filters files that are exempt from the canned no stray whitespace check."""
    filter_list = [
        "third_party/webgpu-headers/webgpu.h.diff",  # Generated diff file
    ]
    return file.LocalPath().replace('\\', '/') not in filter_list


def _CheckCopyrightYear(input_api, output_api, f, new_contents_lines,
                        current_year):
    copyright_regex = re.compile(r'Copyright (\d{4})')
    for line in new_contents_lines:
        if match := copyright_regex.search(line):
            year = int(match.group(1))
            if year != current_year:
                return [
                    output_api.PresubmitPromptWarning(
                        f'{f.LocalPath()}: Copyright year is {year}, should be {current_year} as this is a new file.'
                    )
                ]
            return []
    return [
        output_api.PresubmitPromptWarning(
            f'{f.LocalPath()}: No copyright header found.')
    ]


def _CheckCopyrightText(input_api, output_api, f, new_contents_lines):
    # Find the copyright line and check the lines after it.
    copyright_regex = re.compile(r'Copyright (\d{4})')
    for i, line in enumerate(new_contents_lines):
        if match := copyright_regex.search(line):
            # Determine the comment prefix.
            prefix = "//"
            if line.strip().startswith("#"):
                prefix = "#"

            expected = EXPECTED_LICENSE_TEXT.get(prefix)
            if f.LocalPath().endswith('.go') and prefix == "//":
                expected = EXPECTED_LICENSE_TEXT_GO

            if not expected:
                return []

            # Check the next len(expected) lines.
            actual_license_text = new_contents_lines[i + 1:i + 1 +
                                                     len(expected)]
            if actual_license_text != expected:
                return [
                    output_api.PresubmitPromptWarning(
                        f'{f.LocalPath()}: Copyright license text is incorrect.'
                    )
                ]
            return []
    return []


def _CheckCopyright(input_api, output_api):
    """Checks that newly added files have a correct copyright year and license text."""
    current_year = int(input_api.time.strftime('%Y'))

    errors = []

    # Use a list for deleted contents to handle multiple files with the same
    # content being renamed.
    deleted_files_hashes = []
    for f in input_api.AffectedFiles(include_deletes=True):
        if f.Action() == 'D':
            deleted_files_hashes.append(
                hashlib.sha256(''.join(
                    f.OldContents()).encode('utf-8')).hexdigest())

    for f in input_api.AffectedFiles(include_deletes=False):
        path = f.LocalPath().replace('\\', '/')
        if not (path.endswith(
            ('.h', '.cc', '.cpp', '.mm', '.gn', '.gni', '.cmake',
             'CMakeLists.txt', 'BUILD.bazel', '.py', '.go', '.star', '.js',
             '.mjs', '.sh', '.def', '.idl'))):
            continue

        new_contents_lines = list(f.NewContents())

        # Always check for the copyright license text body if the file is modified.
        errors.extend(
            _CheckCopyrightText(input_api, output_api, f, new_contents_lines))

        # Only check for the copyright year if the file is newly added.
        if f.Action() == 'A':
            new_content_hash = hashlib.sha256(
                ''.join(new_contents_lines).encode('utf-8')).hexdigest()

            # If the file is a rename, we don't check for the copyright year.
            # A rename is detected if a file with the same content is also
            # deleted in the same changelist.
            is_rename = False
            if new_content_hash in deleted_files_hashes:
                deleted_files_hashes.remove(new_content_hash)
                is_rename = True

            if not is_rename:
                errors.extend(
                    _CheckCopyrightYear(input_api, output_api, f,
                                        new_contents_lines, current_year))

    return errors


def CheckPresubmitTests(input_api, output_api):
    """Test PRESUBMIT.py during presubmit."""
    # TODO(crbug.com/517626950): consider running the tests only when
    # PRESUBMIT.py has been modified. For now, this is very cheap, so it
    # doesn't matter.
    return input_api.RunTests(
        input_api.canned_checks.GetUnitTestsInDirectory(
            input_api,
            output_api,
            input_api.PresubmitLocalPath(),
            files_to_check=[r'^PRESUBMIT_test\.py$']))


def CheckChangeTodoHasOwner(input_api, output_api):
    """
    Checks that TODO comments have the issue number.

    This is similar to the canned check in
    depot_tools/presubmit_canned_checks.py but with Dawn-specific exclusions:
    - Excludes DAWN_UNSAFE_TODO macros.
    - Excludes PRESUBMIT.py and PRESUBMIT_test.py to avoid false positives.
    """
    legacyTODO = '\\s*\\(.+\\)\\s*:'  # TODO(owner/bug):
    modernTODO = ':\\s*[^\\s]+\\s*\\-'  # TODO: owner - description
    unowned_todo = input_api.re.compile('\\bTODO(?!(%s|%s))' %
                                        (legacyTODO, modernTODO))

    # Skip PRESUBMIT files to avoid false positives from regexes and tests.
    def FileFilter(affected_file):
        filename = affected_file.LocalPath().replace('\\', '/')
        if filename in ('PRESUBMIT.py', 'PRESUBMIT_test.py'):
            return False
        return input_api.FilterSourceFile(affected_file)

    errors = []
    for f in input_api.AffectedFiles(include_deletes=False,
                                     file_filter=FileFilter):
        for line_num, line in f.ChangedContents():
            if unowned_todo.search(line):
                errors.append(
                    f'Found TODO with no issue number in {f.LocalPath()}:{line_num}'
                )

    if errors:
        return [output_api.PresubmitPromptWarning('\n'.join(errors))]
    return []


def CheckChange(input_api, output_api):
    results = []
    results.extend(
        input_api.canned_checks.CheckForCommitObjects(input_api, output_api))

    result_factory = output_api.PresubmitPromptWarning
    if input_api.is_committing:
        result_factory = output_api.PresubmitError

    # Check for formatting.
    results.extend(
        input_api.canned_checks.CheckPatchFormatted(
            input_api, output_api, result_factory=result_factory))
    results.extend(
        input_api.canned_checks.CheckGNFormatted(input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeHasNoCrAndHasOnlyOneEol(
            input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeHasNoTabs(input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeHasNoStrayWhitespace(
            input_api,
            output_api,
            source_file_filter=_HasNoStrayWhitespaceFilter))
    results.extend(
        input_api.canned_checks.CheckChangeHasDescription(
            input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckDoNotSubmit(input_api, output_api))
    results.extend(_CheckCopyright(input_api, output_api))
    # Note, the verbose_level here should match what is set in tools/lint so
    # the same set of lint errors are reported on the CQ and Kokoro bots.
    results.extend(
        input_api.canned_checks.CheckChangeLintsClean(
            input_api, output_api, lint_filters=LINT_FILTERS, verbose_level=1))

    return results
