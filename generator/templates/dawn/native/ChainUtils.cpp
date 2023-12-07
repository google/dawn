// Copyright 2021 The Dawn & Tint Authors
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

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
#include "{{native_dir}}/ChainUtils.h"

#include <tuple>
#include <unordered_set>
#include <utility>

namespace {{native_namespace}} {

{% set namespace = metadata.namespace %}
MaybeError ValidateSTypes(const ChainedStruct* chain,
                          std::vector<std::vector<{{namespace}}::SType>> oneOfConstraints) {
    std::unordered_set<{{namespace}}::SType> allSTypes;
    for (; chain; chain = chain->nextInChain) {
        DAWN_INVALID_IF(allSTypes.find(chain->sType) != allSTypes.end(),
            "Extension chain has duplicate sType %s.", chain->sType);
        allSTypes.insert(chain->sType);
    }

    for (const auto& oneOfConstraint : oneOfConstraints) {
        bool satisfied = false;
        for ({{namespace}}::SType oneOfSType : oneOfConstraint) {
            if (allSTypes.find(oneOfSType) != allSTypes.end()) {
                DAWN_INVALID_IF(satisfied,
                    "sType %s is part of a group of exclusive sTypes that is already present.",
                    oneOfSType);
                satisfied = true;
                allSTypes.erase(oneOfSType);
            }
        }
    }

    DAWN_INVALID_IF(!allSTypes.empty(), "Unsupported sType %s.", *allSTypes.begin());
    return {};
}

MaybeError ValidateSTypes(const ChainedStructOut* chain,
                          std::vector<std::vector<{{namespace}}::SType>> oneOfConstraints) {
    std::unordered_set<{{namespace}}::SType> allSTypes;
    for (; chain; chain = chain->nextInChain) {
        DAWN_INVALID_IF(allSTypes.find(chain->sType) != allSTypes.end(),
            "Extension chain has duplicate sType %s.", chain->sType);
        allSTypes.insert(chain->sType);
    }

    for (const auto& oneOfConstraint : oneOfConstraints) {
        bool satisfied = false;
        for ({{namespace}}::SType oneOfSType : oneOfConstraint) {
            if (allSTypes.find(oneOfSType) != allSTypes.end()) {
                DAWN_INVALID_IF(satisfied,
                    "sType %s is part of a group of exclusive sTypes that is already present.",
                    oneOfSType);
                satisfied = true;
                allSTypes.erase(oneOfSType);
            }
        }
    }

    DAWN_INVALID_IF(!allSTypes.empty(), "Unsupported sType %s.", *allSTypes.begin());
    return {};
}

// Returns true iff the chain's SType matches the extension, false otherwise. If the SType was
// not already matched, sets the unpacked result accordingly. Otherwise, stores the duplicated
// SType in 'duplicate'.
template <typename Root, typename UnpackedT, typename Ext>
bool UnpackExtension(typename UnpackedT::TupleType& unpacked,
                     typename UnpackedT::BitsetType& bitset,
                     typename UnpackedT::ChainType chain, bool* duplicate) {
    DAWN_ASSERT(chain != nullptr);
    if (chain->sType == STypeFor<Ext>) {
        auto& member = std::get<Ext>(unpacked);
        if (member != nullptr && duplicate) {
            *duplicate = true;
        } else {
            member = reinterpret_cast<Ext>(chain);
            bitset.set(detail::UnpackedIndexOf<UnpackedT, Ext>);
        }
        return true;
    }
    return false;
}

// Tries to match all possible extensions, returning true iff one of the allowed extensions were
// matched, false otherwise. If the SType was not already matched, sets the unpacked result
// accordingly. Otherwise, stores the duplicated SType in 'duplicate'.
template <typename Root, typename UnpackedT, typename AdditionalExts>
struct AdditionalExtensionUnpacker;
template <typename Root, typename UnpackedT, typename... Exts>
struct AdditionalExtensionUnpacker<Root, UnpackedT, detail::AdditionalExtensionsList<Exts...>> {
    static bool Unpack(typename UnpackedT::TupleType& unpacked,
                       typename UnpackedT::BitsetType& bitset,
                       typename UnpackedT::ChainType chain,
                       bool* duplicate) {
        return ((UnpackExtension<Root, UnpackedT, Exts>(unpacked, bitset, chain, duplicate)) ||
                ...);
    }
};

//
// Unpacked chain helpers.
//
{% for type in by_category["structure"] %}
    {% if not type.extensible %}
        {% continue %}
    {% endif %}
    {% set T = as_cppType(type.name) %}
    {% set UnpackedT = "Unpacked<" + T + ">" %}
    template <>
    {{UnpackedT}} Unpack<{{T}}>(typename {{UnpackedT}}::PtrType chain) {
        {{UnpackedT}} result(chain);
        for (typename {{UnpackedT}}::ChainType next = chain->nextInChain;
             next != nullptr;
             next = next->nextInChain) {
            switch (next->sType) {
                {% for extension in type.extensions %}
                    {% set Ext = as_cppType(extension.name) %}
                    case STypeFor<{{Ext}}>: {
                        using ExtPtrType =
                            typename detail::PtrTypeFor<{{UnpackedT}}, {{Ext}}>::Type;
                        std::get<ExtPtrType>(result.mUnpacked) =
                            static_cast<ExtPtrType>(next);
                        result.mBitset.set(
                            detail::UnpackedIndexOf<{{UnpackedT}}, ExtPtrType>
                        );
                        break;
                    }
                {% endfor %}
                default: {
                    using Unpacker =
                        AdditionalExtensionUnpacker<
                            {{T}},
                            {{UnpackedT}},
                            detail::AdditionalExtensions<{{T}}>::List>;
                    Unpacker::Unpack(result.mUnpacked, result.mBitset, next, nullptr);
                    break;
                }
            }
        }
        return result;
    }
    template <>
    ResultOrError<{{UnpackedT}}> ValidateAndUnpack<{{T}}>(typename {{UnpackedT}}::PtrType chain) {
        {{UnpackedT}} result(chain);
        for (typename {{UnpackedT}}::ChainType next = chain->nextInChain;
             next != nullptr;
             next = next->nextInChain) {
            bool duplicate = false;
            switch (next->sType) {
                {% for extension in type.extensions %}
                    {% set Ext = as_cppType(extension.name) %}
                    case STypeFor<{{Ext}}>: {
                        using ExtPtrType =
                            typename detail::PtrTypeFor<{{UnpackedT}}, {{Ext}}>::Type;
                        auto& member = std::get<ExtPtrType>(result.mUnpacked);
                        if (member != nullptr) {
                            duplicate = true;
                        } else {
                            member = static_cast<ExtPtrType>(next);
                            result.mBitset.set(
                                detail::UnpackedIndexOf<{{UnpackedT}}, ExtPtrType>
                            );
                        }
                        break;
                    }
                {% endfor %}
                default: {
                    using Unpacker =
                        AdditionalExtensionUnpacker<
                            {{T}},
                            {{UnpackedT}},
                            detail::AdditionalExtensions<{{T}}>::List>;
                    if (!Unpacker::Unpack(result.mUnpacked,
                                          result.mBitset,
                                          next,
                                          &duplicate)) {
                        return DAWN_VALIDATION_ERROR(
                            "Unexpected chained struct of type %s found on %s chain.",
                            next->sType, "{{T}}"
                        );
                    }
                    break;
                }
            }
            if (duplicate) {
                return DAWN_VALIDATION_ERROR(
                    "Duplicate chained struct of type %s found on %s chain.",
                    next->sType, "{{T}}"
                );
            }
        }
        return result;
    }
{% endfor %}

}  // namespace {{native_namespace}}
