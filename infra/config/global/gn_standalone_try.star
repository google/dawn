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

"""Try Dawn builders using GN and a standalone Dawn checkout (instead of Chromium)."""

load("@chromium-luci//builders.star", "os")
load("@chromium-luci//try.star", "try_")
load("//constants.star", "siso")
# load("//location_filters.star", "exclusion_filters")

try_.defaults.set(
    executable = "recipe:dawn/gn_v2_trybot",
    builder_group = "try",
    bucket = "try",
    pool = "luci.chromium.gpu.try",
    builderless = True,
    build_numbers = True,
    list_view = "try",
    cq_group = "Dawn-CQ",
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_UNTRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
)

# CQ Builders

try_.builder(
    name = "dawn-cq-linux-x64-rel",
    description_html = "Tests Dawn on Linux/x64 on multiple hardware configs. Blocks CL submission",
    max_concurrent_builds = 3,
    os = os.LINUX_DEFAULT,
    ssd = None,
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-sws-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
    # TODO(crbug.com/385317083): Re-add this to the CQ after the recipe is
    # fixed to properly surface failures and the CI builder is green.
    # tryjob = try_.job(
    #     location_filters = exclusion_filters.gn_clang_cq_file_exclusions,
    # ),
)

# Manual trybots

def dawn_linux_manual_builder(*, name, **kwargs):
    return try_.builder(
        name = name,
        max_concurrent_builds = 1,
        os = os.LINUX_DEFAULT,
        ssd = None,
        **kwargs
    )

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-sws-rel",
    description_html = "Tests Dawn on Linux/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-sws-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
)
