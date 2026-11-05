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
	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
)

// shardOf returns the shard, out of m shards, of the test case at position k of
// the listed test cases.
//
// CTS test cases can fall into patterns of longer and shorter running cases.
// To make it unlikely that one shard gets a disproportionate share of the long
// running cases, map a test index to a shard index via a hash function that
// deals well with inputs that form an arithmetic sequence. A Fibonacci hash
// function works particularly well. See Knuth, The Art of Computer Programming,
// Vol. 3, Section 6.4, for intuition and details.
//
// Consider standard multiplicative hashing, mapping an integer k into the range
// {0,...,m-1}. Choose W larger than m, and 'a' relatively prime to W. Then the
// standard multiplicative hash function is:
//
//	h_a(k) = floor( ((a * k) mod W ) / (W / m) )
//	       = floor( ((a * k) mod W ) * (m / W) )
//
// We simplify the math involving W by choosing W to be (1<<32). Then the
// 'mod W' is computed with wrapping arithmetic in uint32, and the division by W
// is computed as a right shift starting with a 64-bit result.
//
// With Fibonacci hashing, choose 'a' to be the odd number closest to W/phi
// where phi is the golden ratio 1.618...
//
// We therefore have pseudo-code:
//
//	const uint32 a = W / phi;
//
//	h_a(k) = ( uint64(uint32(a * k)) * uint64(m) ) >> 32;
func shardOf(k, m int) int {
	const a = 0x9E3779B9 // The odd number closest to W/phi.
	return int((uint64(uint32(k)*a) * uint64(m)) >> 32)
}

// shardCases returns the test cases of shard index out of count, in the order
// they were listed.
func shardCases(cases []common.TestCase, index, count int) []common.TestCase {
	shard := []common.TestCase{}
	for k, testCase := range cases {
		if shardOf(k, count) == index {
			shard = append(shard, testCase)
		}
	}
	return shard
}
