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
#include "{{native_dir}}/ChainUtils_autogen.h"

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
template <typename Root, typename Unpacked, typename Ext>
bool UnpackExtension(Unpacked& unpacked, const ChainedStruct* chain, bool& duplicate) {
    DAWN_ASSERT(chain != nullptr);
    if (chain->sType == STypeFor<Ext>) {
        auto& member = std::get<Ext>(unpacked);
        if (member != nullptr) {
            duplicate = true;
        } else {
            member = reinterpret_cast<Ext>(chain);
        }
        return true;
    }
    return false;
}

// Tries to match all possible extensions, returning true iff one of the allowed extensions were
// matched, false otherwise. If the SType was not already matched, sets the unpacked result
// accordingly. Otherwise, stores the diplicated SType in 'duplicate'.
template <typename Root, typename Unpacked, typename AdditionalExts>
struct AdditionalExtensionUnpacker;
template <typename Root, typename Unpacked, typename... Exts>
struct AdditionalExtensionUnpacker<Root, Unpacked, detail::AdditionalExtensionsList<Exts...>> {
    static bool Unpack(Unpacked& unpacked, const ChainedStruct* chain, bool& duplicate) {
        return ((UnpackExtension<Root, Unpacked, Exts>(unpacked, chain, duplicate)) || ...);
    }
};

//
// Unpacked chain helpers.
//
{% for type in by_category["structure"] %}
    {% if type.extensible == "in" %}
        {% set unpackedChain = "Unpacked" + as_cppType(type.name) + "Chain" %}
        ResultOrError<{{unpackedChain}}> ValidateAndUnpackChain(const {{as_cppType(type.name)}}* chain) {
            const ChainedStruct* next = chain->nextInChain;
            {{unpackedChain}} result;

            for (; next != nullptr; next = next->nextInChain) {
                bool duplicate = false;
                switch (next->sType) {
                    {% for extension in type.extensions %}
                        case STypeFor<{{as_cppType(extension.name)}}>: {
                            auto& member = std::get<const {{as_cppType(extension.name)}}*>(result);
                            if (member != nullptr) {
                                duplicate = true;
                            } else {
                                member = static_cast<const {{as_cppType(extension.name)}}*>(next);
                            }
                            break;
                        }
                    {% endfor %}
                    default: {
                        using Unpacker =
                            AdditionalExtensionUnpacker<
                                {{as_cppType(type.name)}},
                                {{unpackedChain}},
                                detail::AdditionalExtensions<{{as_cppType(type.name)}}>::List>;
                        if (!Unpacker::Unpack(result, next, duplicate)) {
                            return DAWN_VALIDATION_ERROR(
                                "Unexpected chained struct of type %s found on %s chain.",
                                next->sType, "{{as_cppType(type.name)}}"
                            );
                        }
                        break;
                    }
                }
                if (duplicate) {
                    return DAWN_VALIDATION_ERROR(
                        "Duplicate chained struct of type %s found on %s chain.",
                        next->sType, "{{as_cppType(type.name)}}"
                    );
                }
            }
            return result;
        }

    {% endif %}
{% endfor %}

}  // namespace {{native_namespace}}
