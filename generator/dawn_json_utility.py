#!/usr/bin/env python3
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
"""Utilities for processing JSON documentation in Dawn."""

from collections import defaultdict
import json
import re

# ####################################################################################
# Language-Specific Doc Cleaners
# To add support for a new language, create a new `clean_doc_for_...` function
# and add it in the `LANGUAGE_DOC_HANDLER`.
# ####################################################################################


def clean_doc_for_kotlin(doc, valid_names_normalized, enum_names, params=None):
    """Cleans a docstring by validating and formatting references for Kotlin KDoc."""
    if not doc:
        return doc

    kdocs_blocklist = params.get("kdocs_blocklist")
    kdocs_replacements = params.get("kdocs_replacements")

    for old, new in kdocs_replacements.items():
        doc = doc.replace(old, new)

    if any(sub in doc for sub in kdocs_blocklist):
        return ""

    # Example: See @ref WGPUTextureView -> @see WGPUTextureView
    # Normalizes Doxygen-style @ref tags to standard @see tags for KDoc.
    doc = re.sub(r"(?:[Ss]ee\s+)?@ref\s+(\S+)", r"@see \1", doc)

    doc = doc.replace("TODO", "").replace("\n", " ").strip()

    # Example: @see WGPUAdapter -> finds "WGPUAdapter"
    # Extracts all referenced names from @see tags to validate them.
    references = re.findall(r"@see (\S+)", doc)
    for ref in references:
        is_valid = False
        # Convert C++ scope resolution (::) to Kotlin (.).
        if "::" in ref:
            new_ref = ref.replace("::", ".")
            if new_ref.endswith("."):
                new_ref = new_ref.removesuffix(".") + " ."
            doc = doc.replace(ref, new_ref)
            continue

        if ref.startswith("Constants."):
            continue

        ref = ref.replace(".", "").replace(")", "")

        # Handle hyphenated references (e.g., 'my-struct-name').
        if "-" in ref:
            normalized_ref = ref.replace("-", "").lower()
            is_valid = normalized_ref in valid_names_normalized
        # Handle 'wgpu' prefixed references, including enum values.
        elif ref.lower().startswith("wgpu"):
            rest = ref[4:]
            is_valid = "_" in rest or rest.lower() in valid_names_normalized
        # Assume other simple references (e.g., 'MyObject') are valid.
        else:
            is_valid = True

        # Discard docstring if any reference is invalid to avoid incorrect KDoc.
        if not is_valid:
            return ""

    # Example: @see WGPUAdapter -> @see Adapter
    # Strips the standard 'WGPU' or 'wgpu' prefix from type names for cleaner,
    # more idiomatic Kotlin references.
    doc = re.sub(r"@see (?:WGPU|wgpu)(\S+)", r"@see \1", doc)

    # Example: @see WGPUBufferUsage_CopySrc -> @see WGPUBufferUsage.CopySrc
    # Converts C-style enum member references (e.g., EnumName_MemberName) to
    # Kotlin-style qualified references (e.g., EnumName.MemberName).
    def convert_c_style_enum_ref(match):
        enum_name, member_name, trailing_dot = match.groups()
        if enum_name not in enum_names:
            return match.group(0)  # Not a known enum, return original string

        replacement = f"@see {enum_name}.{member_name}"
        if trailing_dot:
            replacement += " ."
        return replacement

    doc = re.sub(r"@see (\w+)_(\w+)\s*(\.?)", convert_c_style_enum_ref, doc)

    return doc


# ####################################################################################
# Core Doc Extraction Logic
# ####################################################################################

TARGETED_CATEGORIES = [
    "structure",
    "enum",
    "bitmask",
    "callback function",
    "function",
    "object",
    "constant",
    "callback info",
]

# Handler of language-specific cleaning functions
LANGUAGE_DOC_HANDLER = {
    "kotlin": clean_doc_for_kotlin,
}


def clean_raw_doc(doc):
    """Performs cleaning of a raw docstring from the loaded data."""
    if not doc or doc.strip() == "TODO":
        return ""
    return doc.replace("\n", " ").strip()


def build_doc_map(by_category, json_data, params=None):
    """Builds a nested, language-agnostic documentation map from JSON data.

  This map contains raw doc strings without any formatting.

  Args:
      by_category: Categorized API data.
      json_data: The raw JSON data.
      params: Language-specific parameters.

  Returns:
      A nested dictionary representing the documentation map.
  """
    if not json_data:
        return {}

    doc_map = defaultdict(lambda: defaultdict(dict))

    data_lookups = {}
    for category, items in json_data.items():
        if not isinstance(items, list):
            continue

        name_to_item_map = {}
        for item in items:
            # Filter out empty or unnamed items
            if item and "name" in item:
                name_to_item_map[item["name"]] = item
        data_lookups[category] = name_to_item_map

    def find_in_data(data_cat, name):
        lookup = data_lookups.get(data_cat, {})
        if name in lookup:
            return lookup[name]

        return None

    def extract_named_docs(dict_list):
        return {
            item["name"]: clean_raw_doc(item.get("doc"))
            for item in dict_list if item and "name" in item and "doc" in item
        }

    def process_common_doc(data_cat, item_name, data_item):
        if "doc" in data_item:
            doc_map[data_cat][item_name]["doc"] = clean_raw_doc(
                data_item["doc"])

    def process_collection(data_cat, item_name, data_item):
        process_common_doc(data_cat, item_name, data_item)
        if "entries" in data_item:
            doc_map[data_cat][item_name]["entries"] = extract_named_docs(
                data_item["entries"])

    category_map = {"enum": "enums", "bitmask": "bitflags"}
    handler_map = {
        "enums": process_collection,
        "bitflags": process_collection,
    }

    for category, items in by_category.items():
        data_category = category_map.get(category)
        if not data_category or data_category not in handler_map:
            continue
        handler = handler_map[data_category]
        for item in items:
            item_name_snake = item.name.snake_case()
            data_item = find_in_data(data_category, item_name_snake)
            if data_item:
                handler(data_category, item.name.get(), data_item)

    return cleanup_doc_map(doc_map=doc_map,
                           by_category=by_category,
                           params=params)


def cleanup_doc_map(doc_map, by_category, params):
    """Post-processes the documentation map to fix and validate cross-references for a specific target language.

  Args:
      doc_map: The raw documentation map from build_doc_map.
      by_category: The categorized API data structure from dawn.json.
      params: Language specific parameters

  Returns:
      The post-processed documentation map.
  """
    if "language" not in params:
        return doc_map

    language = params.get("language")

    cleaner_func = LANGUAGE_DOC_HANDLER.get(language)
    if not cleaner_func:
        return doc_map

    all_valid_names_normalized = set()
    for cat in TARGETED_CATEGORIES:
        for item in by_category.get(cat, []):
            all_valid_names_normalized.add(item.name.CamelCase().lower())

    enum_and_bitflag_names = set()
    for cat in ("enum", "bitmask"):
        for item in by_category.get(cat, []):
            enum_and_bitflag_names.add(item.name.CamelCase())

    # Recursively apply the selected cleaner to the doc_map
    def recursive_clean(d):
        if isinstance(d, dict):
            for k, v in d.items():
                if isinstance(v, (dict, list)):
                    recursive_clean(v)
                elif isinstance(v, str):
                    # Apply the selected language cleaner
                    d[k] = cleaner_func(
                        v,
                        valid_names_normalized=all_valid_names_normalized,
                        enum_names=enum_and_bitflag_names,
                        params=params,
                    )
        elif isinstance(d, list):
            for item in d:
                recursive_clean(item)

    recursive_clean(doc_map)

    return doc_map


def load_json_data(path: str):
    """Loads and parses JSON data from a file."""
    with open(path) as f:
        return json.load(f)
