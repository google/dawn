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

# Starlark file for Buildbucket entries of trybots using the gn_v2_trybot
# recipe. Note that builders must first be defined the the build repo in
# https://source.chromium.org/chromium/infra/infra_superproject/+/main:build/recipes/recipe_modules/dawn/trybots.py

load("//constants.star", "siso")

luci.recipe(
    name = "recipe:dawn/gn_v2_trybot",
    cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
    recipe = "dawn/gn_v2_trybot",
    use_bbagent = True,
    use_python3 = True,
)

LINUX_BUILDER_DIMENSIONS = {
    "pool": "luci.chromium.gpu.try",
    "os": "Ubuntu-22.04",
    "cores": "8",
}

TRY_SERVICE_ACCOUNT = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com"

def generate_properties():
    properties = {
        "$build/siso": {
            "project": siso.project.DEFAULT_UNTRUSTED,
            "remote_jobs": siso.remote_jobs.DEFAULT,
            "configs": ["builder"],
            "enable_cloud_monitoring": True,
            "enable_cloud_profiler": True,
            "enable_cloud_trace": True,
            "metrics_project": "chromium-reclient-metrics",
        },
        "$build/reclient": {
            "instance": siso.project.DEFAULT_UNTRUSTED,
            "jobs": siso.remote_jobs.DEFAULT,
            "metrics_project": "chromium-reclient-metrics",
            "scandeps_server": True,
        },
    }
    return properties

def trybot(name, dimensions):
    """Adds a trybot.

    Note that the mirroring configuration is handled in the build-side
    trybots.py file.

    Args:
        name: The name of the trybot being added.
        dimensions: The Swarming dimensions the trybot should target.
    """
    luci.builder(
        name = name,
        bucket = "try",
        executable = "recipe:dawn/gn_v2_trybot",
        properties = generate_properties(),
        dimensions = dimensions,
        service_account = TRY_SERVICE_ACCOUNT,
    )

trybot("dawn-cq-linux-x64-sws-rel", LINUX_BUILDER_DIMENSIONS)
