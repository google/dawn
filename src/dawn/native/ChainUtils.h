// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_NATIVE_CHAINUTILS_H_
#define SRC_DAWN_NATIVE_CHAINUTILS_H_

#include <bitset>
#include <string>
#include <tuple>

#include "absl/strings/str_format.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Error.h"

namespace dawn::native {

// Tuple type of a Branch and an optional list of corresponding Extensions.
template <typename B, typename... Exts>
struct Branch;

// Typelist type used to specify a list of acceptable Branches.
template <typename... Branches>
struct BranchList;

namespace detail {

// Helpers to get the index in an unpacked tuple type for a particular type.
template <typename Unpacked, typename Ext>
struct UnpackedIndexOf;
template <typename Ext, typename... Exts>
struct UnpackedIndexOf<std::tuple<const Ext*, Exts...>, Ext> {
    static constexpr size_t value = 0;
};
template <typename Ext, typename... Exts>
struct UnpackedIndexOf<std::tuple<Ext, Exts...>, Ext> {
    static constexpr size_t value = 0;
};
template <typename Ext, typename Other, typename... Exts>
struct UnpackedIndexOf<std::tuple<Other, Exts...>, Ext> {
    static constexpr size_t value = 1 + UnpackedIndexOf<std::tuple<Exts...>, Ext>::value;
};

template <typename Unpacked, typename... Exts>
struct UnpackedBitsetForExts {
    // Currently using an internal 64-bit unsigned int for internal representation. This is
    // necessary because std::bitset::operator| is not constexpr until C++23.
    static constexpr auto value = std::bitset<std::tuple_size_v<Unpacked>>(
        ((uint64_t(1) << UnpackedIndexOf<Unpacked, Exts>::value) | ...));
};

template <typename Unpacked, typename...>
struct OneBranchValidator;
template <typename Unpacked, typename B, typename... Exts>
struct OneBranchValidator<Unpacked, Branch<B, Exts...>> {
    static bool Validate(const Unpacked& unpacked,
                         const std::bitset<std::tuple_size_v<Unpacked>>& actual,
                         wgpu::SType& match) {
        // Only check the full bitset when the main branch matches.
        if (std::get<const B*>(unpacked) != nullptr) {
            // Allowed set of extensions includes the branch root as well.
            constexpr auto allowed = UnpackedBitsetForExts<Unpacked, B, Exts...>::value;

            // The configuration is allowed if the actual available chains are a subset.
            if (IsSubset(actual, allowed)) {
                match = STypeFor<B>;
                return true;
            }
        }
        return false;
    }

    static std::string ToString() {
        if constexpr (sizeof...(Exts) > 0) {
            return absl::StrFormat("[ %s -> (%s) ]", STypesToString<B>(),
                                   STypesToString<Exts...>());
        } else {
            return absl::StrFormat("[ %s ]", STypesToString<B>());
        }
    }
};

template <typename Unpacked, typename List>
struct BranchesValidator;
template <typename Unpacked, typename... Branches>
struct BranchesValidator<Unpacked, BranchList<Branches...>> {
    static bool Validate(const Unpacked& unpacked, wgpu::SType& match) {
        // Build a bitset based on which elements in the tuple are actually set. We are essentially
        // just looping over every element in the unpacked tuple, computing the index of the element
        // within the tuple, and setting the respective bit if the element is not nullptr.
        std::bitset<std::tuple_size_v<Unpacked>> actual;
        std::apply(
            [&](const auto*... args) {
                (actual.set(UnpackedIndexOf<Unpacked, decltype(args)>::value, args != nullptr),
                 ...);
            },
            unpacked);

        return ((OneBranchValidator<Unpacked, Branches>::Validate(unpacked, actual, match)) || ...);
    }

    static std::string ToString() {
        return ((absl::StrFormat("  - %s\n", OneBranchValidator<Unpacked, Branches>::ToString())) +
                ...);
    }
};

}  // namespace detail

// Helper to validate that an unpacked chain retrieved via ValidateAndUnpackChain matches a valid
// "branch", where a "branch" is defined as a "root" extension and optional follow-up extensions.
// Returns the wgpu::SType associated with the "root" extension of a "branch" if matched, otherwise
// returns an error.
//
// Example usage:
//     UnpackedChain u;
//     DAWN_TRY_ASSIGN(u, ValidateAndUnpackChain(desc));
//     wgpu::SType rootType;
//     DAWN_TRY_ASSIGN(rootType,
//         ValidateBranches<BranchList<Branch<Root1>, Branch<Root2, R2Ext1>>>(u));
//     switch (rootType) {
//         case Root1: {
//             <do something>
//         }
//         case Root2: {
//             R2Ext1 ext = std::get<const R2Ext1*>(u);
//             if (ext) {
//                 <do something with optional extension(s)>
//             }
//         }
//         default:
//             UNREACHABLE();
//     }
//
// The example above checks that the unpacked chain is either:
//     - only a Root1 extension
//     - or a Root2 extension with an optional R2Ext1 extension
// Any other configuration is deemed invalid.
template <typename Branches, typename Unpacked>
ResultOrError<wgpu::SType> ValidateBranches(const Unpacked& unpacked) {
    using Validator = detail::BranchesValidator<Unpacked, Branches>;

    wgpu::SType match = wgpu::SType::Invalid;
    if (Validator::Validate(unpacked, match)) {
        return match;
    }
    return DAWN_VALIDATION_ERROR(
        "Expected chain root to match one of the following branch types with optional extensions:\n"
        "%sInstead found: %s",
        Validator::ToString(), detail::UnpackedChainToString(unpacked));
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CHAINUTILS_H_
