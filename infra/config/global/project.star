#
# Copyright 2023 The Dawn Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

def _milestone_details(*, chromium_project, ref, platforms):
    """Define the details for an active milestone.

    Args:
      * chromium_project - The name of the LUCI project that is configured for the
        milestone.
      * ref - The ref in the Dawn git repository that contains the code for the
        milestone.
      * platforms - A list of platform strings that the milestone is active for.
    """
    return struct(
        chromium_project = chromium_project,
        ref = ref,
        platforms = platforms,
    )

ACTIVE_MILESTONES = {
    m["name"]: _milestone_details(
        chromium_project = m["chromium_project"],
        ref = m["ref"],
        platforms = m["platforms"],
    )
    for m in json.decode(io.read_file("./milestones.json")).values()
}
