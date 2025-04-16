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

# Starlark file for Buildbucket entries of CI builders using the gn_v2 recipe.
# Note that builders must first be defined the the build repo in
# https://source.chromium.org/chromium/infra/infra_superproject/+/main:build/recipes/recipe_modules/dawn/builders.py

load("//constants.star", "siso")

luci.recipe(
    name = "recipe:dawn/gn_v2",
    cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
    recipe = "dawn/gn_v2",
    use_bbagent = True,
    use_python3 = True,
)

LINUX_BUILDER_DIMENSIONS = {
    "pool": "luci.chromium.gpu.ci",
    "os": "Ubuntu-22.04",
    "cores": "8",
}

CI_SERVICE_ACCOUNT = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com"
CI_SHADOW_SERVICE_ACCOUNT = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com"

def generate_properties_for_project(project):
    properties = {
        "builder_group": "dawn",
        "$build/siso": {
            "project": project,
            "remote_jobs": siso.remote_jobs.DEFAULT,
            "configs": ["builder"],
            "enable_cloud_monitoring": True,
            "enable_cloud_profiler": True,
            "enable_cloud_trace": True,
            "metrics_project": "chromium-reclient-metrics",
        },
        "$build/reclient": {
            "instance": project,
            "jobs": siso.remote_jobs.DEFAULT,
            "metrics_project": "chromium-reclient-metrics",
            "scandeps_server": True,
        },
    }
    return properties

def parent_builder(name, dimensions, **kwargs):
    """Adds a CI parent builder.

    Args:
      name: The name of the builder.
      dimensions: The Swarming dimensions the builder should target.
    """
    luci.builder(
        name = name,
        bucket = "ci",
        # TODO(crbug.com/385317083): Switch this to be triggered by
        # "primary-poller" once we're ready to begin migration to this recipe.
        triggered_by = None,
        schedule = "triggered",
        executable = "recipe:dawn/gn_v2",
        dimensions = dimensions,
        properties = generate_properties_for_project(siso.project.DEFAULT_TRUSTED),
        shadow_properties = generate_properties_for_project(siso.project.DEFAULT_UNTRUSTED),
        # TODO(crbug.com/385317083): Make CI builders notify the gardeners once
        # migration to this recipe has begun.
        service_account = CI_SERVICE_ACCOUNT,
        shadow_service_account = CI_SHADOW_SERVICE_ACCOUNT,
    )

def child_tester(name, parent_builder):
    """Adds a CI child tester.

    Args:
        name: The name of the tester.
        parent_builder: The name of the parent builder that will trigger the
            tester.
    """
    luci.builder(
        name = name,
        bucket = "ci",
        triggered_by = [parent_builder],
        executable = "recipe:dawn/gn_v2",
        dimensions = {
            "pool": "luci.chromium.gpu.ci",
            "os": "Ubuntu-22.04",
            "cores": "2",
        },
        properties = {
            "builder_group": "dawn",
        },
        service_account = CI_SERVICE_ACCOUNT,
        shadow_service_account = CI_SHADOW_SERVICE_ACCOUNT,
    )

parent_builder("dawn-linux-x64-builder-rel", LINUX_BUILDER_DIMENSIONS)

child_tester("dawn-linux-x64-sws-rel", "ci/dawn-linux-x64-builder-rel")
