# Copyright 2025 The Dawn & Tint Authors
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

"""Legacy builder definitions that were originally in main.star.

Moved due to them causing issues while working on crbug.com/385317083. Any
builders that will be kept after that migration should be grouped and moved into
appropriately named files.
"""

load("//constants.star", "siso")
load("//location_filters.star", "exclusion_filters")
load("//project.star", "ACTIVE_MILESTONES")

os_category = struct(
    LINUX = "Linux",
    MAC = "Mac",
    WINDOWS = "Windows",
)

def os_enum(category, console_name):
    return struct(category = category, console_name = console_name)

os = struct(
    LINUX = os_enum(os_category.LINUX, "linux"),
    MAC = os_enum(os_category.MAC, "mac"),
    WINDOWS = os_enum(os_category.WINDOWS, "win"),
)

def get_dimension(os):
    """Returns the dimension to use for the input os.

    Args:
        os: An os enum to check against.

    Returns:
        A string containing the dimensions the given OS should target.
    """
    if os.category == os_category.LINUX:
        return "Ubuntu-24.04"
    elif os.category == os_category.MAC:
        return "Mac-12|Mac-13|Mac-14|Mac-15"
    elif os.category == os_category.WINDOWS:
        return "Windows-10"

    return "Invalid Dimension"

luci.notifier(
    name = "gardener-notifier",
    notify_rotation_urls = [
        "https://chrome-ops-rotation-proxy.appspot.com/current/grotation:webgpu-gardener",
    ],
    on_occurrence = ["FAILURE", "INFRA_FAILURE"],
)

# Recipes

def get_os_from_arg(arg):
    """Get OS enum for a builder name string

    Args:
      arg: builder name string to get enum for

    Returns:
      An OS enum struct

    """

    if arg.find("linux") != -1:
        return os.LINUX
    if arg.find("win") != -1:
        return os.WINDOWS
    if arg.find("mac") != -1:
        return os.MAC
    return os.MAC

def get_default_caches(os):
    """Get standard caches for builders

    Args:
      os: OS enum for the builder

    Returns:
      A list of caches
    """
    caches = []
    if os.category == os_category.MAC:
        # Cache for mac_toolchain tool and XCode.app
        caches.append(swarming.cache(name = "osx_sdk", path = "osx_sdk"))

    return caches

def get_default_dimensions(os):
    """Get dimensions for a builder that don't depend on being CI vs Try

    Args:
      os: OS enum for the builder

    Returns:
      A dimension dict
    """
    dimensions = {}

    # We have 32bit test configurations but some of our toolchain is 64bit (like CIPD)
    dimensions["cpu"] = "x86-64"
    dimensions["os"] = get_dimension(os)

    return dimensions

def get_common_properties(os, clang, rbe_project, remote_jobs):
    """Add the common properties for a builder that don't depend on being CI vs Try

    Args:
      os: OS enum for the builder
      clang: A boolean denoting whether the builder is using clang or not
      rbe_project: A string containing the RBE project to use
      remote_jobs: An int specifying how many remote jobs to use when compiling

    Returns:
      A properties dict
    """
    properties = {}
    msvc = os.category == os_category.WINDOWS and not clang

    properties = {
        "$build/siso": {
            "configs": ["builder"],
            "enable_cloud_monitoring": True,
            "enable_cloud_profiler": True,
            "enable_cloud_trace": True,
            "project": rbe_project,
        },
    }
    if not msvc:
        reclient_props = {
            "instance": rbe_project,
            "jobs": remote_jobs,
            "scandeps_server": True,
        }
        properties["$build/reclient"] = reclient_props
        properties["$build/siso"]["remote_jobs"] = remote_jobs

    return properties

def add_ci_cmake_builder(name, os, properties):
    """Add a CI CMake builder

    Args:
      name: builder's name in string form
      os: OS enum for the builder
      properties: properties dictionary
    """
    clang = properties["clang"]
    fuzzer = ("gen_fuzz_corpus" in properties) and properties["gen_fuzz_corpus"]

    dimensions_ci = get_default_dimensions(os)
    dimensions_ci["pool"] = "luci.flex.ci"
    properties_ci = get_common_properties(
        os,
        clang,
        siso.project.DEFAULT_TRUSTED,
        siso.remote_jobs.DEFAULT,
    )

    # TODO(crbug.com/343503161): Remove sheriff_rotations after SoM is updated.
    properties_ci["sheriff_rotations"] = ["dawn"]
    properties_ci["gardener_rotations"] = ["dawn"]
    properties_ci.update(properties)
    shadow_properties_ci = get_common_properties(
        os,
        clang,
        siso.project.DEFAULT_UNTRUSTED,
        siso.remote_jobs.DEFAULT,
    )
    shadow_properties_ci.update(properties)
    schedule_ci = None
    if fuzzer:
        schedule_ci = "0 0 0 * * * *"
    triggered_by_ci = None
    if not fuzzer:
        triggered_by_ci = ["primary-poller"]
    luci.builder(
        name = name,
        bucket = "ci",
        schedule = schedule_ci,
        triggered_by = triggered_by_ci,
        executable = "recipe:dawn/cmake",
        properties = properties_ci,
        dimensions = dimensions_ci,
        caches = get_default_caches(os),
        notifies = ["gardener-notifier"],
        service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
        shadow_service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        shadow_properties = shadow_properties_ci,
    )

def add_try_cmake_builder(name, os, properties):
    """Add a Try CMake builder

    Args:
      name: builder's name in string form
      os: OS enum for the builder
      properties: properties dictionary
    """
    clang = properties["clang"]

    dimensions_try = get_default_dimensions(os)
    dimensions_try["pool"] = "luci.flex.try"
    properties_try = get_common_properties(
        os,
        clang,
        siso.project.DEFAULT_UNTRUSTED,
        siso.remote_jobs.LOW_JOBS_FOR_CQ,
    )
    properties_try.update(properties)
    luci.builder(
        name = name,
        bucket = "try",
        executable = "recipe:dawn/cmake",
        properties = properties_try,
        dimensions = dimensions_try,
        caches = get_default_caches(os),
        service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    )

def dawn_cmake_standalone_builder(name, clang, debug, cpu, asan, ubsan, experimental = False):
    """Adds both the CI and Try standalone builders as appropriate for the CMake build

    Args:
      name: builder's name in string form
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      asan: is this builder building with asan enabled
      ubsan: is this builder building with ubsan enabled
      experimental: is this builder experimental
    """
    os = get_os_from_arg(name)

    properties = {
        "asan": asan,
        "clang": clang,
        "debug": debug,
        "target_cpu": cpu,
        "ubsan": ubsan,
    }

    add_ci_cmake_builder(name, os, properties)
    add_try_cmake_builder(name, os, properties)

    config = ""
    if clang:
        config = "clang"
    elif os.category == os_category.WINDOWS:
        config = "msvc"

    category = ""
    category += os.console_name

    if os.category != os_category.MAC:
        category += "|" + config
        if config != "msvc":
            category += "|dbg" if debug else "|rel"

    short_name = "dbg" if debug else "rel"
    if os.category != os_category.MAC:
        if config != "msvc":
            short_name = cpu

    luci.console_view_entry(
        console_view = "ci",
        builder = "ci/" + name,
        category = category,
        short_name = short_name,
    )

    luci.list_view_entry(
        list_view = "try",
        builder = "try/" + name,
    )

    # Only automatically add CQ verifiers for non-ASAN and non-UBSAN bots to
    # minimize CQ load.
    if not asan and not ubsan:
        luci.cq_tryjob_verifier(
            experiment_percentage = 100 if experimental else None,
            cq_group = "Dawn-CQ",
            builder = "dawn:try/" + name,
            location_filters = exclusion_filters.cmake_cq_file_exclusions,
        )
    else:
        luci.cq_tryjob_verifier(
            experiment_percentage = 100 if experimental else None,
            cq_group = "Dawn-CQ",
            builder = "dawn:try/" + name,
            includable_only = True,
        )

    # These builders run fine unbranched on branch CLs, so add them to the
    # branch groups as well.
    for milestone in ACTIVE_MILESTONES.keys():
        luci.cq_tryjob_verifier(
            experiment_percentage = 100,  # Temporarily make this experimental
            cq_group = "Dawn-CQ-" + milestone,
            builder = "dawn:try/" + name,
        )

def clang_tidy_dawn_tryjob():
    """Adds a tryjob that runs clang tidy on new patchset upload."""
    luci.cq_tryjob_verifier(
        cq_group = "Dawn-CQ",
        builder = "chromium:try/tricium-clang-tidy",
        owner_whitelist = ["project-dawn-tryjob-access"],
        experiment_percentage = 100,
        disable_reuse = True,
        mode_allowlist = [cq.MODE_NEW_PATCHSET_RUN],
        location_filters = [
            cq.location_filter(path_regexp = r".+\.h"),
            cq.location_filter(path_regexp = r".+\.c"),
            cq.location_filter(path_regexp = r".+\.cc"),
            cq.location_filter(path_regexp = r".+\.cpp"),
        ],
    )

luci.list_view_entry(
    list_view = "try",
    builder = "try/presubmit",
)

luci.builder(
    name = "presubmit",
    bucket = "try",
    executable = "recipe:run_presubmit",
    dimensions = {
        "cpu": "x86-64",
        "os": get_dimension(os.LINUX),
        "pool": "luci.flex.try",
    },
    properties = {
        "repo_name": "dawn",
        "runhooks": True,
    },
    service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
)

luci.builder(
    name = "cts-roller",
    bucket = "ci",
    # Run at 5 UTC on weekdays - which is 10pm PST
    schedule = "0 5 * * 1-5",
    executable = "recipe:dawn/roll_cts",
    execution_timeout = 9 * time.hour,
    dimensions = {
        "cpu": "x86-64",
        "os": get_dimension(os.LINUX),
        "pool": "luci.flex.ci",
    },
    properties = {
        "gardener_rotations": ["dawn"],
        "repo_name": "dawn",
        "runhooks": True,
        # TODO(crbug.com/343503161): Remove sheriff_rotations after SoM is updated.
        "sheriff_rotations": ["dawn"],
    },
    caches = [
        swarming.cache("golang"),
        swarming.cache("gocache"),
        swarming.cache("nodejs"),
        swarming.cache("npmcache"),
    ],
    notifies = ["gardener-notifier"],
    service_account = "dawn-automated-expectations@chops-service-accounts.iam.gserviceaccount.com",
)

luci.console_view_entry(
    console_view = "ci",
    builder = "ci/cts-roller",
    category = "cron|roll",
    short_name = "cts",
)

# The following standalone builders have been replaced with functionally
# equivalent ones using the gn_v2 recipe. See crbug.com/385317083.
# * cron-linux-clang-rel-x64
#   * dawn-linux-x64-sws-clusterfuzz
# * linux-clang-dbg-x64
#   * dawn-cq-linux-x64-dbg
#   * dawn-cq-linux-x64-fuzz-dbg
# * linux-clang-dbg-x86
#   * dawn-cq-linux-x86-dbg
#   * dawn-cq-linux-x86-fuzz-dbg
# * linux-clang-rel-x64
#   * dawn-cq-linux-x64-rel
#   * dawn-cq-linux-x64-fuzz-rel
# * linux-clang-rel-x86
#   * dawn-cq-linux-x86-rel
#   * dawn-cq-linux-x86-fuzz-rel
# * mac-dbg
#   * dawn-cq-mac-x64-dbg
# * mac-rel
#   * dawn-cq-mac-x64-rel
# * win-clang-dbg-x64
#   * dawn-cq-win-x64-dbg
# * win-clang-dbg-x86
#   * dawn-cq-win-x86-dbg
# * win-clang-rel-x64
#   * dawn-cq-win-x64-rel
# * win-clang-rel-x86
#   * dawn-cq-win-x86-rel
# * win-msvc-dbg-x64
#   * dawn-cq-win-x64-msvc-dbg
# * win-msvc-rel-x64
#   * dawn-cq-win-x64-msvc-rel

# The following CMake builders have been replaced with functionally equivalent
# ones defined using chromium-luci code. See crbug.com/459517292.
# * cmake-linux-clang-dbg-x64
#   * dawn-linux-x64-sws-cmake-dbg
#   * dawn-cq-linux-x64-sws-cmake-rel
# * cmake-linux-clang-rel-x64
#   * dawn-linux-x64-sws-cmake-rel
#   * dawn-cq-linux-x64-sws-cmake-rel

# The following CMake builders have been removed due to deciding that they were
# not providing value in go/dawn-standalone-builders-dd.
# * cmake-linux-clang-dbg-x64-asan
# * cmake-linux-clang-dbg-x64-ubsan

dawn_cmake_standalone_builder("cmake-linux-clang-rel-x64-asan", clang = True, debug = False, cpu = "x64", asan = True, ubsan = False)
dawn_cmake_standalone_builder("cmake-linux-clang-rel-x64-ubsan", clang = True, debug = False, cpu = "x64", asan = False, ubsan = True)
dawn_cmake_standalone_builder("cmake-mac-dbg", clang = True, debug = True, cpu = "x64", asan = False, ubsan = False, experimental = False)
dawn_cmake_standalone_builder("cmake-mac-rel", clang = True, debug = False, cpu = "x64", asan = False, ubsan = False, experimental = False)
dawn_cmake_standalone_builder("cmake-win-msvc-dbg-x64", clang = False, debug = True, cpu = "x64", asan = False, ubsan = False)
dawn_cmake_standalone_builder("cmake-win-msvc-rel-x64", clang = False, debug = False, cpu = "x64", asan = False, ubsan = False)

clang_tidy_dawn_tryjob()

# CQ

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_max_burst = 4,
    submit_burst_delay = 480 * time.second,
    gerrit_listener_type = cq.GERRIT_LISTENER_TYPE_LEGACY_POLLER,
)

def _create_dawn_cq_group(name, refs, refs_exclude = None):
    luci.cq_group(
        name = name,
        watch = cq.refset(
            "https://dawn.googlesource.com/dawn",
            refs = refs,
            refs_exclude = refs_exclude,
        ),
        acls = [
            acl.entry(
                acl.CQ_COMMITTER,
                groups = "project-dawn-submit-access",
            ),
            acl.entry(
                acl.CQ_DRY_RUNNER,
                groups = "project-dawn-tryjob-access",
            ),
            acl.entry(
                acl.CQ_NEW_PATCHSET_RUN_TRIGGERER,
                groups = "project-dawn-tryjob-access",
            ),
        ],
        verifiers = [
            luci.cq_tryjob_verifier(
                builder = "dawn:try/presubmit",
                disable_reuse = True,
            ),
        ],
        retry_config = cq.retry_config(
            single_quota = 1,
            global_quota = 2,
            failure_weight = 1,
            transient_failure_weight = 1,
            timeout_weight = 2,
        ),
        user_limit_default = cq.user_limit(
            name = "default-limit",
            run = cq.run_limits(max_active = 4),
        ),
    )

def _create_branch_groups():
    for milestone, details in ACTIVE_MILESTONES.items():
        _create_dawn_cq_group(
            "Dawn-CQ-" + milestone,
            [details.ref],
        )

_create_dawn_cq_group(
    "Dawn-CQ",
    ["refs/heads/.+"],
    [details.ref for details in ACTIVE_MILESTONES.values()],
)
_create_branch_groups()
