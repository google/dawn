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
"""Lookup table (LUT)."""

from dataclasses import dataclass
from typing import Generic, Optional, Sequence, TypeVar

T = TypeVar("T")


@dataclass
class Offset:
    """Mutable integer offset into a LUT, updated in-place by LUT.compact()."""

    value: int

    def __index__(self) -> int:
        return self.value

    def __str__(self) -> str:
        return str(self.value)

    def __eq__(self, other: object) -> bool:
        if isinstance(other, Offset):
            return self.value == other.value
        if isinstance(other, int):
            return self.value == other
        return NotImplemented


@dataclass
class _Sequence:
    """A span of entries in the lookup table."""

    offset: Offset
    count: int


class LUT(Generic[T]):
    """A lookup table that compresses indexed sequence data."""

    def __init__(self) -> None:
        self._storage: list[T] = []
        self._sequences: list[_Sequence] = []

    def add(self, items: Sequence[T]) -> Optional[Offset]:
        """Adds a sequence of items to the table.

        The sequence of items stored at [offset, offset+N), where N is the
        number of items added, will remain equal even after calling compact().
        Returns an Offset holding the start index in the list, or None if
        items is empty.
        """
        if not items:
            return None

        offset = Offset(len(self._storage))
        count = len(items)
        self._storage.extend(items)
        self._sequences.append(_Sequence(offset=offset, count=count))
        return offset

    def compact(self) -> list[T]:
        """Reorders the table items so that the table storage is compacted.

        Shuffles data around and deduplicates sequences of common data.
        Updates Offset objects returned by add() in-place.
        """
        # Generate integer identifiers for each unique item in the table.
        # We use these to compare items instead of comparing the real data as
        # this function is comparison-heavy, and integer compares are cheap.
        src_ids = self._item_ids()
        dst_ids = [0] * len(src_ids)

        # Make a copy of the data held in the table; use the copy as the source
        # and self._storage as the destination.
        src_data = list(self._storage)
        dst_data = self._storage

        # Sort all the sequences by length, with the largest first.
        # This helps 'seed' the compacted form with the largest items first.
        self._sequences.sort(key=lambda s: s.count, reverse=True)

        # unplaced is the list of sequences that have not yet been placed.
        unplaced = list(self._sequences)

        # placed is the list of sequences that have been placed.
        placed: list[_Sequence] = []

        def place_sequence(i: int) -> None:
            placed.append(unplaced.pop(i))

        def cp(dst_offset: int, src_offset: int, count: int) -> None:
            dst_data[dst_offset:dst_offset +
                     count] = src_data[src_offset:src_offset + count]
            dst_ids[dst_offset:dst_offset +
                    count] = src_ids[src_offset:src_offset + count]

        new_size = 0

        while unplaced:
            # Place the next largest unplaced sequence at the end of the list.
            first = unplaced[0]
            cp(new_size, first.offset.value, first.count)
            first.offset.value = new_size
            new_size += first.count
            place_sequence(0)

            while True:
                # Look for the unplaced sequence with the longest contiguous
                # overlap against the currently placed data. Every overlapping
                # element must match (any mismatch disqualifies that alignment),
                # which means a match can either:
                # 1. Match the entire candidate sequence against a slice inside
                #    the placed data (full containment).
                # 2. Match a prefix of the candidate against a suffix of the
                #    placed data, extending the placed data on the right.
                # 3. Match a suffix of the candidate against a prefix of the
                #    placed data, extending the placed data on the left.
                best_dst = 0
                best_src: Optional[_Sequence] = None
                best_len = 0
                best_idx = 0

                for i, seq in enumerate(unplaced):
                    if best_len >= seq.count:
                        # The best match is already at least as long as this
                        # sequence and sequences are sorted by size, so best
                        # cannot be beaten. Stop searching.
                        break

                    seq_offset = seq.offset.value
                    # Perform a full sweep from left to right, scoring the match.
                    for shift in range(best_len - seq.count + 1,
                                       new_size - best_len):
                        dst_s = shift if shift > 0 else 0
                        dst_e = (shift + seq.count if shift +
                                 seq.count < new_size else new_size)
                        count = dst_e - dst_s

                        if best_len < count:
                            src_s = seq_offset - (shift if shift < 0 else 0)
                            src_e = src_s + count
                            if src_ids[src_s:src_e] == dst_ids[dst_s:dst_e]:
                                best_dst = shift
                                best_src = seq
                                best_len = count
                                best_idx = i

                if best_src is None:
                    # Nothing matched. Not even one element.
                    # Resort to placing the next largest sequence at the end.
                    break

                if best_dst < 0:
                    # Best match wants to place the sequence to the left of the
                    # current output. We have to shuffle everything...
                    n = -best_dst
                    dst_data[n:n + new_size] = dst_data[:new_size]
                    dst_ids[n:n + new_size] = dst_ids[:new_size]
                    new_size += n
                    best_dst = 0
                    for p in placed:
                        p.offset.value += n

                # Place the best matching sequence.
                cp(best_dst, best_src.offset.value, best_src.count)
                if best_dst + best_src.count > new_size:
                    new_size = best_dst + best_src.count
                best_src.offset.value = best_dst
                place_sequence(best_idx)

        # Shrink the output buffer to the new size.
        del dst_data[new_size:]
        return dst_data

    def _item_ids(self) -> list[int]:
        """Generates a list of integer IDs for all unique items in storage."""
        keys: list[int] = []
        data_to_key: dict[T, int] = {}
        try:
            for data in self._storage:
                key = data_to_key.get(data)
                if key is None:
                    key = len(data_to_key)
                    data_to_key[data] = key
                keys.append(key)
        except TypeError:
            # Fallback if items are comparable but not hashable.
            keys.clear()
            unique_items: list[T] = []
            for data in self._storage:
                for idx, existing in enumerate(unique_items):
                    if existing == data:
                        keys.append(idx)
                        break
                else:
                    keys.append(len(unique_items))
                    unique_items.append(data)
        return keys
