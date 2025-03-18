// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
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

#include "dawn/common/Sha3.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

// All test vectors are taken from
// https://csrc.nist.gov/projects/cryptographic-standards-and-guidelines/example-values

// Check that the internal Keccak function computes the correct update for the state.
// The test vectors are taken from intermediate results in
// https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHA3-384_1600.pdf
TEST(Sha3, CheckKeccak) {
    // Initial state and the end of page 2.
    Sha3State initial = {
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000,
    };

    // State after one round of Keccak page 38.
    Sha3State expected = {
        0x160a71930ca2a8a3, 0x51ddb3b2c251d927, 0xdc9aa0e4bcc6ad64, 0xf8a14407bd495cd1,
        0xf17b866f4d85efd0, 0x82d81704476a121c, 0x7cfa31f915062efa, 0x247ec6d41845f643,
        0x1dc24a64defcf173, 0xdcfed5e4a19d5574, 0xbcae93fcba107740, 0x65c3c92d3e365e0c,
        0x368cad882027066c, 0xea7e1cd53bf20792, 0x0daa532dce42e107, 0x2cb333822af5990b,
        0x8b2bb75c6d627045, 0x4f550b11db349feb, 0x8bd8bdb493854b0c, 0x5386b7c92346dcc6,
        0xcc3f58bf9c237342, 0xb3658927a749b560, 0x294fd2abede42c19, 0xb3fb1448aef7d65c,
        0x644f5649d63f5404,
    };

    Sha3State transformed = KeccakForTesting(initial);

    for (size_t i = 0; i < transformed.size(); i++) {
        EXPECT_EQ(expected[i], transformed[i]);
    }
}

}  // anonymous namespace
}  // namespace dawn
