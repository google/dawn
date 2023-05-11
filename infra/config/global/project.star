#
# Copyright 2023 The Dawn Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

def _milestone_details(*, chromium_project, ref):
  """Define the details for an active milestone.

  Args:
    * chromium_project - The name of the LUCI project that is configured for the
      milestone.
    * ref - The ref in the Dawn git repository that contains the code for the
      milestone.
  """
  return struct(
      chromium_project = chromium_project,
      ref = ref,
  )

ACTIVE_MILESTONES = {
    m["name"]: _milestone_details(
        chromium_project = m["chromium_project"], ref = m["ref"])
        for m in json.decode(io.read_file("./milestones.json")).values()
}
