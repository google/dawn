#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
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
"""Tests for tools/tint_gen/lut.py (ported from tools/src/lut/lut_test.go)."""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import lut


class LutTest(unittest.TestCase):

    def test_compact_with_fragments(self) -> None:
        table: lut.LUT[str] = lut.LUT()
        indices = [
            table.add(list("the life in your")),
            table.add(list("in your life that count")),
            table.add(list("In the end,")),
            table.add(list("the life in")),
            table.add(list("count. It's the")),
            table.add(list("years")),
            table.add(list("in your years.")),
            table.add(list("it's not the years in")),
            table.add(list("not the years")),
            table.add(list("not the years in your")),
            table.add(list("end, it's")),
        ]

        chars = table.compact()

        expect = (
            "In the end, it's not the years in your life that count. It's the"
            " life in your years.")
        got = "".join(chars)
        self.assertEqual(got, expect)

        expected_indices = [
            61,  # the life in your
            31,  # in your life that count
            0,  # In the end,
            61,  # the life in
            49,  # count. It's the
            25,  # years
            70,  # in your years.
            12,  # it's not the years in
            17,  # not the years
            17,  # not the years in your
            7,  # end, it's
        ]
        for i, p in enumerate(indices):
            self.assertIsNotNone(p)
            assert p is not None
            self.assertEqual(
                p.value,
                expected_indices[i],
                f"Index {i} was not expected. Expected {expected_indices[i]},"
                f" got {p.value}",
            )

    def test_empty_sequence_returns_none(self) -> None:
        table: lut.LUT[int] = lut.LUT()
        self.assertIsNone(table.add([]))
        self.assertEqual(table.compact(), [])


if __name__ == "__main__":
    unittest.main()
