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

"""CI Dawn builders using GN and a standalone Dawn checkout (instead of Chromium)."""

load("@chromium-luci//builder_config.star", "builder_config")
load("@chromium-luci//builders.star", "os")
load("@chromium-luci//ci.star", "ci")
load("@chromium-luci//consoles.star", "consoles")
load("@chromium-luci//gn_args.star", "gn_args")
load("//constants.star", "siso")

ci.defaults.set(
    executable = "recipe:dawn/gn_v2",
    builder_group = "ci",
    bucket = "ci",
    pool = "luci.chromium.gpu.ci",
    triggered_by = ["primary-poller"],
    build_numbers = True,
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
    shadow_service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_TRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
    thin_tester_cores = 2,
)

ci.builder(
    name = "dawn-linux-x64-builder-dbg",
    description_html = "Compile debug Dawn test binaries for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_node_bindings",
            "dawn_swiftshader",
            "linux_clang",
            "component",
            "debug",
            "x64",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|dbg",
        short_name = "x64",
    ),
)

ci.builder(
    name = "dawn-linux-x64-builder-rel",
    description_html = "Compiles release Dawn test binaries for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_node_bindings",
            "dawn_swiftshader",
            "linux_clang",
            "component",
            "release",
            "x64",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel",
        short_name = "x64",
    ),
)

ci.builder(
    name = "dawn-linux-x64-fuzz-dbg",
    description_html = "Compiles and runs debug Dawn binaries for 'tools/run fuzz' for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "libfuzzer",
            "linux_clang",
            "non_component",
            "debug",
            "x64",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|dbg|fuzz",
        short_name = "x64",
    ),
)

ci.builder(
    name = "dawn-linux-x86-builder-rel",
    description_html = "Compiles release Dawn test binaries for Linux/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            # dawn_node is intentionally omitted since the original standalone
            # Linux/x86/Clang builders did not build/test with node.
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_swiftshader",
            "linux_clang",
            "component",
            "release",
            "x86",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel",
        short_name = "x86",
    ),
)

ci.builder(
    name = "dawn-linux-x64-fuzz-rel",
    description_html = "Compiles and runs release Dawn binaries for 'tools/run fuzz' for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "libfuzzer",
            "linux_clang",
            "non_component",
            "release",
            "x64",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel|fuzz",
        short_name = "x64",
    ),
)

ci.builder(
    name = "dawn-linux-x86-fuzz-rel",
    description_html = "Compiles and runs release Dawn binaries for 'tools/run fuzz' for Linux/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "libfuzzer",
            "linux_clang",
            "non_component",
            "release",
            "x86",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel|fuzz",
        short_name = "x86",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-sws-dbg",
    description_html = "Tests debug Dawn on Linux/x64 with SwiftShader",
    parent = "dawn-linux-x64-builder-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|dbg|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-sws-rel",
    description_html = "Tests release Dawn on Linux/x64 with SwiftShader",
    parent = "dawn-linux-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x86-sws-rel",
    description_html = "Tests release Dawn on Linux/x86 with SwiftShader",
    parent = "dawn-linux-x86-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x86",
        short_name = "sws",
    ),
)
