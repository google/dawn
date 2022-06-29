#!/usr/bin/env lucicfg
#
# Copyright 2021 The Dawn Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""
main.star: lucicfg configuration for Dawn's standalone builers.
"""

# Use LUCI Scheduler BBv2 names and add Scheduler realms configs.
lucicfg.enable_experiment("crbug.com/1182002")

luci.builder.defaults.experiments.set({
    "luci.recipes.use_python3": 100,
})

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

def get_default_properties(os, clang, debug, cpu, fuzzer):
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
        goma_props = {}
        goma_props.update({
            "server_host": "goma.chromium.org",
            "rpc_extra_params": "?prod",
            "use_luci_auth": True,
        })
        if os.category != os_category.MAC:
            goma_props["enable_ats"] = True
        properties["$build/goma"] = goma_props

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
    properties_ci = get_default_properties(os, clang, debug, cpu, fuzzer)
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
    properties_try = get_default_properties(os, clang, debug, cpu, fuzzer)
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

def chromium_dawn_tryjob(os):
    """Adds a tryjob that tests against Chromium

    Args:
      os: string for the OS, should be one or linux|mac|win
    """
    luci.cq_tryjob_verifier(
        cq_group = "Dawn-CQ",
        builder = "chromium:try/" + os + "-dawn-rel",
    )

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

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-win10-x86-rel",
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

luci.cq_group(
    name = "Dawn-CQ",
    watch = cq.refset(
        "https://dawn.googlesource.com/dawn",
        refs = ["refs/heads/.+"],
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
