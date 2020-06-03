# Copyright 2020 The Tint Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Presubmit script for Tint.
See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""


def _LicenseHeader(input_api):
    """Returns the license header regexp."""
    # Accept any year number from 2019 to the current year
    current_year = int(input_api.time.strftime('%Y'))
    allowed_years = (str(s) for s in reversed(xrange(2019, current_year + 1)))
    years_re = '(' + '|'.join(allowed_years) + ')'
    license_header = (
        r'.*? Copyright( \(c\))? %(year)s The Tint [Aa]uthors\n '
        r'.*?\n'
        r'.*? Licensed under the Apache License, Version 2.0 (the "License");\n'
        r'.*? you may not use this file except in compliance with the License.\n'
        r'.*? You may obtain a copy of the License at\n'
        r'.*?\n'
        r'.*?     http://www.apache.org/licenses/LICENSE-2.0\n'
        r'.*?\n'
        r'.*? Unless required by applicable law or agreed to in writing, software\n'
        r'.*? distributed under the License is distributed on an "AS IS" BASIS,\n'
        r'.*? WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        r'.*? See the License for the specific language governing permissions and\n'
        r'.*? limitations under the License.\n') % {
            'year': years_re,
        }
    return license_header


def CheckChange(input_api, output_api):
    results = []

    results += input_api.canned_checks.CheckChangeHasDescription(
        input_api, output_api)
    results += input_api.canned_checks.CheckPatchFormatted(input_api,
                                                           output_api,
                                                           check_python=True)
    results += input_api.canned_checks.CheckGNFormatted(input_api, output_api)
    results += input_api.canned_checks.CheckChangeHasNoCrAndHasOnlyOneEol(
        input_api, output_api)
    results += input_api.canned_checks.CheckChangeHasNoTabs(
        input_api, output_api)
    results += input_api.canned_checks.CheckChangeTodoHasOwner(
        input_api, output_api)
    results += input_api.canned_checks.CheckChangeHasNoStrayWhitespace(
        input_api, output_api)
    results += input_api.canned_checks.CheckDoNotSubmit(input_api, output_api)
    results += input_api.canned_checks.CheckChangeLintsClean(
        input_api, output_api)
    results += input_api.canned_checks.CheckGenderNeutral(
        input_api, output_api)

    return results


def CheckChangeOnUpload(input_api, output_api):
    return CheckChange(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
    return CheckChange(input_api, output_api)
