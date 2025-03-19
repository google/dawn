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

// Check that the correct values are computed in all by all the hash functions on the 1600 bit test
// vectors for each SHA3 version. (we don't use the other ones because they are not byte aligned).
TEST(Sha3, AllSizesOn1600BitTestVector) {
    std::array<uint64_t, 25> message = {
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3, 0xa3a3a3a3a3a3a3a3,
        0xa3a3a3a3a3a3a3a3,
    };

    Sha3_224::Output sha224Expected = {0x93, 0x76, 0x81, 0x6A, 0xBA, 0x50, 0x3F, 0x72, 0xF9, 0x6C,
                                       0xE7, 0xEB, 0x65, 0xAC, 0x09, 0x5D, 0xEE, 0xE3, 0xBE, 0x4B,
                                       0xF9, 0xBB, 0xC2, 0xA1, 0xCB, 0x7E, 0x11, 0xE0};

    Sha3_224::Output sha224Actual = Sha3_224::Hash(&message, sizeof(message));
    for (size_t i = 0; i < sha224Actual.size(); i++) {
        EXPECT_EQ(sha224Actual[i], sha224Expected[i]);
    }

    Sha3_256::Output sha256Expected = {0x79, 0xF3, 0x8A, 0xDE, 0xC5, 0xC2, 0x03, 0x07,
                                       0xA9, 0x8E, 0xF7, 0x6E, 0x83, 0x24, 0xAF, 0xBF,
                                       0xD4, 0x6C, 0xFD, 0x81, 0xB2, 0x2E, 0x39, 0x73,
                                       0xC6, 0x5F, 0xA1, 0xBD, 0x9D, 0xE3, 0x17, 0x87};

    Sha3_256::Output sha256Actual = Sha3_256::Hash(&message, sizeof(message));
    for (size_t i = 0; i < sha256Actual.size(); i++) {
        EXPECT_EQ(sha256Actual[i], sha256Expected[i]);
    }

    Sha3_384::Output sha384Expected = {0x18, 0x81, 0xDE, 0x2C, 0xA7, 0xE4, 0x1E, 0xF9, 0x5D, 0xC4,
                                       0x73, 0x2B, 0x8F, 0x5F, 0x00, 0x2B, 0x18, 0x9C, 0xC1, 0xE4,
                                       0x2B, 0x74, 0x16, 0x8E, 0xD1, 0x73, 0x26, 0x49, 0xCE, 0x1D,
                                       0xBC, 0xDD, 0x76, 0x19, 0x7A, 0x31, 0xFD, 0x55, 0xEE, 0x98,
                                       0x9F, 0x2D, 0x70, 0x50, 0xDD, 0x47, 0x3E, 0x8F};

    Sha3_384::Output sha384Actual = Sha3_384::Hash(&message, sizeof(message));
    for (size_t i = 0; i < sha384Actual.size(); i++) {
        EXPECT_EQ(sha384Actual[i], sha384Expected[i]);
    }

    Sha3_512::Output sha512Expected = {
        0xE7, 0x6D, 0xFA, 0xD2, 0x20, 0x84, 0xA8, 0xB1, 0x46, 0x7F, 0xCF, 0x2F, 0xFA,
        0x58, 0x36, 0x1B, 0xEC, 0x76, 0x28, 0xED, 0xF5, 0xF3, 0xFD, 0xC0, 0xE4, 0x80,
        0x5D, 0xC4, 0x8C, 0xAE, 0xEC, 0xA8, 0x1B, 0x7C, 0x13, 0xC3, 0x0A, 0xDF, 0x52,
        0xA3, 0x65, 0x95, 0x84, 0x73, 0x9A, 0x2D, 0xF4, 0x6B, 0xE5, 0x89, 0xC5, 0x1C,
        0xA1, 0xA4, 0xA8, 0x41, 0x6D, 0xF6, 0x54, 0x5A, 0x1C, 0xE8, 0xBA, 0x00};

    Sha3_512::Output sha512Actual = Sha3_512::Hash(&message, sizeof(message));
    for (size_t i = 0; i < sha512Actual.size(); i++) {
        EXPECT_EQ(sha512Actual[i], sha512Expected[i]);
    }
}

// Check that the correct values are computed by various non-1600 sizes for one of the variants
TEST(Sha3, VariousMessageSizes) {
    // Test what happens with no data provided.
    {
        Sha3_224::Output sha224Expected = {
            0x6B, 0x4E, 0x03, 0x42, 0x36, 0x67, 0xDB, 0xB7, 0x3B, 0x6E, 0x15, 0x45, 0x4F, 0x0E,
            0xB1, 0xAB, 0xD4, 0x59, 0x7F, 0x9A, 0x1B, 0x07, 0x8E, 0x3F, 0x5B, 0x5A, 0x6B, 0xC7};

        Sha3_224::Output sha224Actual = Sha3_224::Hash(nullptr, 0);
        for (size_t i = 0; i < sha224Actual.size(); i++) {
            EXPECT_EQ(sha224Actual[i], sha224Expected[i]);
        }
    }

    // Test any amount of data that doesn't require any Keccak other than the final one.
    {
        std::array<char, 42> data;
        data.fill('a');

        Sha3_224::Output sha224Expected = {
            0xF4, 0xFB, 0x9A, 0x8F, 0xA2, 0x04, 0x0F, 0x8C, 0x72, 0xC2, 0x2D, 0x6B, 0x26, 0xEE,
            0xE5, 0xA9, 0xF2, 0x2F, 0xC4, 0x9B, 0x86, 0x01, 0xC6, 0xCD, 0xAB, 0xE2, 0x3E, 0x0D};

        Sha3_224::Output sha224Actual = Sha3_224::Hash(&data, sizeof(data));
        for (size_t i = 0; i < sha224Actual.size(); i++) {
            EXPECT_EQ(sha224Actual[i], sha224Expected[i]);
        }
    }

    // Test a "long" message that requires many internal Keccaks
    {
        std::array<char, 4000> data;
        data.fill('a');

        Sha3_224::Output sha224Expected = {
            0xB5, 0xBF, 0x09, 0x7A, 0x65, 0xB3, 0xC9, 0x42, 0x72, 0x6E, 0x16, 0x77, 0xD8, 0xEE,
            0x5A, 0x59, 0x39, 0x6F, 0xD2, 0xEB, 0xBF, 0xB4, 0xCB, 0x2E, 0x9F, 0xBA, 0x2F, 0x67};

        Sha3_224::Output sha224Actual = Sha3_224::Hash(&data, sizeof(data));
        for (size_t i = 0; i < sha224Actual.size(); i++) {
            EXPECT_EQ(sha224Actual[i], sha224Expected[i]);
        }
    }
}

// Check that using Update at any byte boundary produces the same result
TEST(Sha3, UpdateAnyByteBoundary) {
    // 500 bytes need at least three different Keccak transforms so we should cover all the special
    // values of the internal insertion offset in Sha3.
    std::array<char, 500> data;
    data.fill('a');

    Sha3_224::Output sha224Expected = {0xB3, 0x36, 0x37, 0x5B, 0xF8, 0xE6, 0x2A, 0x2F, 0x34, 0xF8,
                                       0xC4, 0xE3, 0xF9, 0xFD, 0xD5, 0x34, 0x1E, 0x85, 0xB0, 0x23,
                                       0x49, 0xD9, 0x18, 0x73, 0xC5, 0x9D, 0x70, 0x36};

    for (size_t offset = 0; offset < sizeof(data); offset++) {
        Sha3_224 sha;
        sha.Update(&data[0], offset);
        sha.Update(&data[offset], sizeof(data) - offset);
        Sha3_224::Output sha224Actual = sha.Finalize();

        for (size_t i = 0; i < sha224Actual.size(); i++) {
            EXPECT_EQ(sha224Actual[i], sha224Expected[i]);
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
