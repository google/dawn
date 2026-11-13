// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package node

import (
	"fmt"
	"slices"
	"strconv"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
)

func TestShardOfFollowsTheGoldenRatio(t *testing.T) {
	// floor(frac(k * 0.618...) * 2) for k = 0..7.
	want := []int{0, 1, 0, 1, 0, 0, 1, 0}
	for k, shard := range want {
		if got := shardOf(k, 2); got != shard {
			t.Errorf("shardOf(%d, 2) = %d, want %d", k, got, shard)
		}
	}
}

// Every test case must be in exactly one shard, each shard must keep the listed
// order, and the shards must be close to the same size.
func TestShardsPartitionTheTestCasesInOrder(t *testing.T) {
	const numCases = 1000
	cases := make([]common.TestCase, numCases)
	for i := range cases {
		cases[i] = common.TestCase(fmt.Sprint(i))
	}

	for count := 1; count <= 40; count++ {
		// The shard each test case was found in, or -1 if it has not been found yet.
		shardOfCase := slices.Repeat([]int{-1}, numCases)
		for index := range count {
			shard := shardCases(cases, index, count)

			previous := -1
			for _, testCase := range shard {
				position, _ := strconv.Atoi(string(testCase))
				if position <= previous {
					t.Errorf("count %d: shard %d is not in listed order", count, index)
				}
				previous = position

				if shardOfCase[position] != -1 {
					t.Errorf("count %d: test case %d is in shards %d and %d", count, position, shardOfCase[position], index)
				}
				shardOfCase[position] = index
			}

			// Shard sizes differ by a few test cases at most.
			if size := len(shard); size < numCases/count-2 || size > numCases/count+3 {
				t.Errorf("count %d: shard %d has %d test cases", count, index, size)
			}
		}

		if missing := slices.Index(shardOfCase, -1); missing != -1 {
			t.Errorf("count %d: test case %d is in no shard", count, missing)
		}
	}
}
