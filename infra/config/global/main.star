#!/usr/bin/env lucicfg
#
# Copyright 2021 The Dawn & Tint Authors
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

"""
main.star: lucicfg configuration for Dawn's standalone builers.
"""

load("//project.star", "ACTIVE_MILESTONES")

# Use LUCI Scheduler BBv2 names and add Scheduler realms configs.
lucicfg.enable_experiment("crbug.com/1182002")

lucicfg.config(fail_on_warnings = True)

luci.project(
    name = "dawn",
    buildbucket = "cr-buildbucket.appspot.com",
    logdog = "luci-logdog.appspot.com",
    milo = "luci-milo.appspot.com",
    notify = "luci-notify.appspot.com",
    scheduler = "luci-scheduler.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        acl.entry(
            roles = [
                acl.PROJECT_CONFIGS_READER,
                acl.LOGDOG_READER,
                acl.BUILDBUCKET_READER,
                acl.SCHEDULER_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            roles = [
                acl.SCHEDULER_OWNER,
            ],
            groups = "project-dawn-admins",
        ),
        acl.entry(
            roles = [
                acl.LOGDOG_WRITER,
            ],
            groups = "luci-logdog-chromium-writers",
        ),
    ],
    bindings = [
        luci.binding(
            roles = "role/configs.validator",
            users = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        ),
        luci.binding(
            roles = "role/swarming.taskServiceAccount",
            users = "dawn-automated-expectations@chops-service-accounts.iam.gserviceaccount.com",
        ),
    ],
)

luci.logdog(gs_bucket = "chromium-luci-logdog")

luci.bucket(
    name = "ci",
    acls = [
        acl.entry(
            roles = [
                acl.BUILDBUCKET_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            acl.BUILDBUCKET_TRIGGERER,
        ),
    ],
)

# Allow LED users to trigger swarming tasks directly when debugging ci
# builders.
luci.binding(
    realm = "ci",
    roles = "role/swarming.taskTriggerer",
    groups = "flex-ci-led-users",
)

luci.bucket(
    name = "try",
    acls = [
        acl.entry(
            acl.BUILDBUCKET_TRIGGERER,
            groups = [
                "project-dawn-tryjob-access",
                "service-account-cq",
            ],
        ),
    ],
)

# Allow LED users to trigger swarming tasks directly when debugging try
# builders.
luci.binding(
    realm = "try",
    roles = "role/swarming.taskTriggerer",
    groups = "flex-try-led-users",
)

os_category = struct(
    LINUX = "Linux",
    MAC = "Mac",
    WINDOWS = "Windows",
)

def os_enum(dimension, category, console_name):
    return struct(dimension = dimension, category = category, console_name = console_name)

os = struct(
    LINUX = os_enum("Ubuntu-18.04", os_category.LINUX, "linux"),
    MAC = os_enum("Mac-10.15|Mac-11", os_category.MAC, "mac"),
    WINDOWS = os_enum("Windows-10", os_category.WINDOWS, "win"),
)

reclient = struct(
    instance = struct(
        DEFAULT_TRUSTED = "rbe-chromium-trusted",
        DEFAULT_UNTRUSTED = "rbe-chromium-untrusted",
    ),
    jobs = struct(
        HIGH_JOBS_FOR_CI = 250,
        LOW_JOBS_FOR_CQ = 150,
    ),
)


# Recipes

def get_builder_executable():
    """Get standard executable for builders

    Returns:
      A luci.recipe
    """
    return luci.recipe(
        name = "dawn",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        cipd_version = "refs/heads/main",
    )

def get_presubmit_executable():
    """Get standard executable for presubmit

    Returns:
      A luci.recipe
    """
    return luci.recipe(
        name = "run_presubmit",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        cipd_version = "refs/heads/main",
    )

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

def get_default_caches(os, clang):
    """Get standard caches for builders

    Args:
      os: OS enum for the builder
      clang: is this builder running clang

    Returns:
      A list of caches
    """
    caches = []
    if os.category == os_category.WINDOWS and clang:
        caches.append(swarming.cache(name = "win_toolchain", path = "win_toolchain"))
    elif os.category == os_category.MAC:
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
    dimensions["os"] = os.dimension

    return dimensions

def get_default_properties(os, clang, debug, cpu, fuzzer, reclient_instance, reclient_jobs):
    """Get the properties for a builder that don't depend on being CI vs Try

    Args:
      os: OS enum for the builder
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      fuzzer: is this builder running the fuzzers

    Returns:
      A properties dict
    """
    properties = {}

    properties["debug"] = debug
    properties["target_cpu"] = cpu

    properties["clang"] = clang
    msvc = os.category == os_category.WINDOWS and not clang

    if fuzzer:
        properties["gen_fuzz_corpus"] = True

    if not msvc:
        reclient_props = {
            "instance": reclient_instance,
            "jobs": reclient_jobs,
            "metrics_project": "chromium-reclient-metrics",
            "scandeps_server": True
        }
        properties["$build/reclient"] = reclient_props

    return properties

def add_ci_builder(name, os, clang, debug, cpu, fuzzer):
    """Add a CI builder

    Args:
      name: builder's name in string form
      os: OS enum for the builder
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      fuzzer: is this builder running the fuzzers
    """
    dimensions_ci = get_default_dimensions(os)
    dimensions_ci["pool"] = "luci.flex.ci"
    properties_ci = get_default_properties(os, clang, debug, cpu, fuzzer,
                                           reclient.instance.DEFAULT_TRUSTED,
                                           reclient.jobs.HIGH_JOBS_FOR_CI)
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
        executable = get_builder_executable(),
        properties = properties_ci,
        dimensions = dimensions_ci,
        caches = get_default_caches(os, clang),
        service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
    )

def add_try_builder(name, os, clang, debug, cpu, fuzzer):
    """Add a Try builder

    Args:
      name: builder's name in string form
      os: OS enum for the builder
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      fuzzer: is this builder running the fuzzers
    """
    dimensions_try = get_default_dimensions(os)
    dimensions_try["pool"] = "luci.flex.try"
    properties_try = get_default_properties(os, clang, debug, cpu, fuzzer,
                                            reclient.instance.DEFAULT_UNTRUSTED,
                                            reclient.jobs.LOW_JOBS_FOR_CQ)
    properties_try["$depot_tools/bot_update"] = {
        "apply_patch_on_gclient": True,
    }
    luci.builder(
        name = name,
        bucket = "try",
        executable = get_builder_executable(),
        properties = properties_try,
        dimensions = dimensions_try,
        caches = get_default_caches(os, clang),
        service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    )

def dawn_standalone_builder(name, clang, debug, cpu, fuzzer = False):
    """Adds both the CI and Try standalone builders as appropriate

    Args:
      name: builder's name in string form
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      fuzzer: enable building fuzzer corpus

    """
    os = get_os_from_arg(name)

    add_ci_builder(name, os, clang, debug, cpu, fuzzer)
    if not fuzzer:
        add_try_builder(name, os, clang, debug, cpu, fuzzer)

    config = ""
    if clang:
        config = "clang"
    elif os.category == os_category.WINDOWS:
        config = "msvc"

    category = ""
    if fuzzer:
        category += "cron|"
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

    if not fuzzer:
        luci.list_view_entry(
            list_view = "try",
            builder = "try/" + name,
        )

        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ",
            builder = "dawn:try/" + name,
        )

        # These builders run fine unbranched on branch CLs, so add them to the
        # branch groups as well.
        for milestone in ACTIVE_MILESTONES.keys():
            luci.cq_tryjob_verifier(
                cq_group = "Dawn-CQ-" + milestone,
                builder = "dawn:try/" + name,
            )

def _add_branch_verifiers(builder_name, os, min_milestone = None, includable_only = False):
    for milestone, details in ACTIVE_MILESTONES.items():
        if os not in details.platforms:
            continue
        if min_milestone != None and int(milestone[1:]) < min_milestone:
            continue
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ-" + milestone,
            builder = "{}:try/{}".format(details.chromium_project, builder_name),
            includable_only = includable_only,
        )

# We use the DEPS version for branches because ToT builders do not make sense on
# branches and the DEPS versions already exist.
_os_arch_to_branch_builder = {
    "linux": "dawn-linux-x64-deps-rel",
    "mac": "dawn-mac-x64-deps-rel",
    "win": "dawn-win10-x64-deps-rel",
    "android-arm": "dawn-android-arm-deps-rel",
    "android-arm64": "dawn-android-arm64-deps-rel",
}

# The earliest milestone that the builder is relevant for
_os_arch_to_min_milestone = {
    "linux": 112,
    "mac": 112,
    "win": 112,
    "android-arm": None,
    "android-arm64": None,
}

def chromium_dawn_tryjob(os, arch = None):
    """Adds a tryjob that tests against Chromium

    Args:
      os: string for the OS, should be one or linux|mac|win
      arch: string for the arch, or None
    """

    if arch:
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ",
            builder = "chromium:try/{os}-dawn-{arch}-rel".format(os = os, arch = arch),
        )
        _add_branch_verifiers(
            _os_arch_to_branch_builder["{os}-{arch}".format(os = os, arch = arch)],
            os,
            _os_arch_to_min_milestone["{os}-{arch}".format(os = os, arch = arch)],
        )
    else:
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ",
            builder = "chromium:try/{}-dawn-rel".format(os),
        )
        _add_branch_verifiers(_os_arch_to_branch_builder[os], os)

luci.gitiles_poller(
    name = "primary-poller",
    bucket = "ci",
    repo = "https://dawn.googlesource.com/dawn",
    refs = [
        "refs/heads/main",
    ],
)

luci.list_view_entry(
    list_view = "try",
    builder = "try/presubmit",
)

luci.builder(
    name = "presubmit",
    bucket = "try",
    executable = get_presubmit_executable(),
    dimensions = {
        "cpu": "x86-64",
        "os": os.LINUX.dimension,
        "pool": "luci.flex.try",
    },
    properties = {
        "repo_name": "dawn",
        "runhooks": True,
        "$depot_tools/bot_update": {
            "apply_patch_on_gclient": True,
        },
    },
    service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
)

luci.builder(
    name = "cts-roller",
    bucket = "ci",
    # Run at 5 UTC - which is 10pm PST
    schedule = "0 5 * * *",
    executable = luci.recipe(
        name = "dawn/roll_cts",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        cipd_version = "refs/heads/main",
    ),
    execution_timeout = 9 * time.hour,
    dimensions = {
        "cpu": "x86-64",
        "os": os.LINUX.dimension,
        "pool": "luci.flex.ci",
    },
    properties = {
        "repo_name": "dawn",
        "runhooks": True,
    },
    caches = [
        swarming.cache("golang"),
        swarming.cache("gocache"),
        swarming.cache("nodejs"),
        swarming.cache("npmcache"),
    ],
    notifies = [
        luci.notifier(
            name = "cts-roller-notifier",
            # TODO(dawn:1940): Switch to the rotation email when stable
            # notify_rotation_urls = [
            #     "https://chrome-ops-rotation-proxy.appspot.com/current/grotation:webgpu-gardener",
            # ],
            notify_emails = ["enga@chromium.org"],
            # TODO(dawn:1940): Remove SUCCESS when stable
            on_occurrence = ["SUCCESS", "FAILURE", "INFRA_FAILURE"],
        )
    ],
    service_account = "dawn-automated-expectations@chops-service-accounts.iam.gserviceaccount.com",
)

luci.console_view_entry(
    console_view = "ci",
    builder = "ci/cts-roller",
    category = "cron|roll",
    short_name = "cts",
)

#                        name, clang, debug, cpu(, fuzzer)
dawn_standalone_builder("linux-clang-dbg-x64", True, True, "x64")
dawn_standalone_builder("linux-clang-dbg-x86", True, True, "x86")
dawn_standalone_builder("linux-clang-rel-x64", True, False, "x64")
dawn_standalone_builder("linux-clang-rel-x86", True, False, "x86")
dawn_standalone_builder("mac-dbg", True, True, "x64")
dawn_standalone_builder("mac-rel", True, False, "x64")
dawn_standalone_builder("win-clang-dbg-x64", True, True, "x64")
dawn_standalone_builder("win-clang-dbg-x86", True, True, "x86")
dawn_standalone_builder("win-clang-rel-x64", True, False, "x64")
dawn_standalone_builder("win-clang-rel-x86", True, False, "x86")
dawn_standalone_builder("win-msvc-dbg-x64", False, True, "x64")
dawn_standalone_builder("win-msvc-rel-x64", False, False, "x64")
dawn_standalone_builder("cron-linux-clang-rel-x64", True, False, "x64", True)

chromium_dawn_tryjob("linux")
chromium_dawn_tryjob("mac")
chromium_dawn_tryjob("win")
chromium_dawn_tryjob("android", "arm")
chromium_dawn_tryjob("android", "arm64")

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-win10-x86-rel",
    includable_only = True,
)
_add_branch_verifiers("dawn-win10-x86-deps-rel", "win", includable_only = True)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-mac-arm64-rel",
    includable_only = True,
)

# Views

luci.milo(
    logo = "https://storage.googleapis.com/chrome-infra-public/logo/dawn-logo.png",
)

luci.console_view(
    name = "ci",
    title = "Dawn CI Builders",
    repo = "https://dawn.googlesource.com/dawn",
    refs = ["refs/heads/main"],
)

luci.list_view(
    name = "try",
    title = "Dawn try Builders",
)

# CQ

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_max_burst = 4,
    submit_burst_delay = 480 * time.second,
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
                groups = "project-dawn-committers",
            ),
            acl.entry(
                acl.CQ_DRY_RUNNER,
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
