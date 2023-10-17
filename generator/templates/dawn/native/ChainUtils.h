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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
#ifndef {{DIR}}_CHAIN_UTILS_H_
#define {{DIR}}_CHAIN_UTILS_H_

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace = metadata.namespace %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set prefix = metadata.proc_table_prefix.lower() %}
#include <tuple>
#include <type_traits>
#include <unordered_set>

#include "absl/strings/str_format.h"
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/Error.h"
#include "{{native_dir}}/{{namespace}}_structs_autogen.h"

namespace {{native_namespace}} {

namespace detail {

    // SType for implementation details. Kept inside the detail namespace for extensibility.
    template <typename T>
    inline {{namespace}}::SType STypeForImpl;

    // Specialize STypeFor to map from native struct types to their SType.
    {% for value in types["s type"].values %}
        {% if value.valid and value.name.get() in types %}
            template <>
            constexpr inline {{namespace}}::SType STypeForImpl<{{as_cppEnum(value.name)}}> = {{namespace}}::SType::{{as_cppEnum(value.name)}};
        {% endif %}
    {% endfor %}

    template <typename Arg, typename... Rest>
    std::string STypesToString() {
        if constexpr (sizeof...(Rest)) {
            return absl::StrFormat("%s, ", STypeForImpl<Arg>) + STypesToString<Rest...>();
        } else {
            return absl::StrFormat("%s", STypeForImpl<Arg>);
        }
    }

    //
    // Unpacked chain types structs and helpers.
    //   Note that unpacked types are tuples to enable further templating extensions based on
    //   typing via something like std::get<const Extension*> in templated functions.
    //

    // Typelist type used to further add extensions to chain roots when they are not in the json.
    template <typename... Exts>
    struct AdditionalExtensionsList;

    // Root specializations for adding additional extensions.
    template <typename Root>
    struct AdditionalExtensions {
        using List = AdditionalExtensionsList<>;
    };

    // Template structs to get the typing for the unpacked chains.
    template <typename...>
    struct UnpackedChain;
    template <typename... Additionals, typename... Ts>
    struct UnpackedChain<AdditionalExtensionsList<Additionals...>, Ts...> {
        using Type = std::tuple<Ts..., Additionals...>;
    };

    // Template function that returns a string of the non-nullptr STypes from an unpacked chain.
    template <typename Unpacked>
    std::string UnpackedChainToString(const Unpacked& unpacked) {
        std::string result = "( ";
        std::apply(
            [&](const auto*... args) {
                (([&](const auto* arg) {
                    if (arg != nullptr) {
                        // reinterpret_cast because this chained struct might be forward-declared
                        // without a definition. The definition may only be available on a
                        // particular backend.
                        const auto* chainedStruct = reinterpret_cast<const wgpu::ChainedStruct*>(arg);
                        result += absl::StrFormat("%s, ", chainedStruct->sType);
                    }
                }(args)), ...);}, unpacked);
        result += " )";
        return result;
    }

}  // namespace detail

    template <typename T>
    constexpr inline wgpu::SType STypeFor = detail::STypeForImpl<T>;
    template <typename T>
    constexpr inline wgpu::SType STypeFor<const T*> = detail::STypeForImpl<T>;

    template <typename T>
    void FindInChain(const ChainedStruct* chain, const T** out) {
        for (; chain; chain = chain->nextInChain) {
            if (chain->sType == STypeFor<T>) {
                *out = static_cast<const T*>(chain);
                break;
            }
        }
    }
    template <typename T>
    void FindInChain(ChainedStructOut* chain, T** out) {
        for (; chain; chain = chain->nextInChain) {
            if (chain->sType == STypeFor<T>) {
                *out = static_cast<T*>(chain);
                break;
            }
        }
    }

    // Verifies that |chain| only contains ChainedStructs of types enumerated in
    // |oneOfConstraints| and contains no duplicate sTypes. Each vector in
    // |oneOfConstraints| defines a set of sTypes that cannot coexist in the same chain.
    // For example:
    //   ValidateSTypes(chain, { { ShaderModuleSPIRVDescriptor, ShaderModuleWGSLDescriptor } }))
    //   ValidateSTypes(chain, { { Extension1 }, { Extension2 } })
    MaybeError ValidateSTypes(const ChainedStruct* chain,
                              std::vector<std::vector<{{namespace}}::SType>> oneOfConstraints);
    MaybeError ValidateSTypes(const ChainedStructOut* chain,
                              std::vector<std::vector<{{namespace}}::SType>> oneOfConstraints);

    template <typename T>
    MaybeError ValidateSingleSTypeInner(const ChainedStruct* chain, T sType) {
        DAWN_INVALID_IF(chain->sType != sType,
            "Unsupported sType (%s). Expected (%s)", chain->sType, sType);
        return {};
    }
    template <typename T>
    MaybeError ValidateSingleSTypeInner(const ChainedStructOut* chain, T sType) {
        DAWN_INVALID_IF(chain->sType != sType,
            "Unsupported sType (%s). Expected (%s)", chain->sType, sType);
        return {};
    }

    template <typename T, typename... Args>
    MaybeError ValidateSingleSTypeInner(const ChainedStruct* chain, T sType, Args... sTypes) {
        if (chain->sType == sType) {
            return {};
        }
        return ValidateSingleSTypeInner(chain, sTypes...);
    }
    template <typename T, typename... Args>
    MaybeError ValidateSingleSTypeInner(const ChainedStructOut* chain, T sType, Args... sTypes) {
        if (chain->sType == sType) {
            return {};
        }
        return ValidateSingleSTypeInner(chain, sTypes...);
    }

    // Verifies that |chain| contains a single ChainedStruct of type |sType| or no ChainedStructs
    // at all.
    template <typename T>
    MaybeError ValidateSingleSType(const ChainedStruct* chain, T sType) {
        if (chain == nullptr) {
            return {};
        }
        DAWN_INVALID_IF(chain->nextInChain != nullptr,
            "Chain can only contain a single chained struct.");
        return ValidateSingleSTypeInner(chain, sType);
    }
    template <typename T>
    MaybeError ValidateSingleSType(const ChainedStructOut* chain, T sType) {
        if (chain == nullptr) {
            return {};
        }
        DAWN_INVALID_IF(chain->nextInChain != nullptr,
            "Chain can only contain a single chained struct.");
        return ValidateSingleSTypeInner(chain, sType);
    }

    // Verifies that |chain| contains a single ChainedStruct with a type enumerated in the
    // parameter pack or no ChainedStructs at all.
    template <typename T, typename... Args>
    MaybeError ValidateSingleSType(const ChainedStruct* chain, T sType, Args... sTypes) {
        if (chain == nullptr) {
            return {};
        }
        DAWN_INVALID_IF(chain->nextInChain != nullptr,
            "Chain can only contain a single chained struct.");
        return ValidateSingleSTypeInner(chain, sType, sTypes...);
    }
    template <typename T, typename... Args>
    MaybeError ValidateSingleSType(const ChainedStructOut* chain, T sType, Args... sTypes) {
        if (chain == nullptr) {
            return {};
        }
        DAWN_INVALID_IF(chain->nextInChain != nullptr,
            "Chain can only contain a single chained struct.");
        return ValidateSingleSTypeInner(chain, sType, sTypes...);
    }

    // Template type to get root type from the unpacked chain and vice-versa.
    template <typename Unpacked>
    struct RootTypeFor;
    template <typename Root>
    struct UnpackedTypeFor;

}  // namespace {{native_namespace}}

// Include specializations before declaring types for ordering purposes.
#include "{{native_dir}}/ChainUtilsImpl.inl"

namespace {{native_namespace}} {

    {% for type in by_category["structure"] %}
        {% if type.extensible == "in" %}
            {% set unpackedChain = "Unpacked" + as_cppType(type.name) + "Chain" %}
            using {{unpackedChain}} = detail::UnpackedChain<
                detail::AdditionalExtensions<{{as_cppType(type.name)}}>::List{{ "," if len(type.extensions) != 0 else ""}}
                {% for extension in type.extensions %}
                    const {{as_cppType(extension.name)}}*{{ "," if not loop.last else "" }}
                {% endfor %}
            >::Type;
            template <>
            struct UnpackedTypeFor<{{as_cppType(type.name)}}> {
                using Type = {{unpackedChain}};
            };
            ResultOrError<{{unpackedChain}}> ValidateAndUnpackChain(const {{as_cppType(type.name)}}* chain);

        {% endif %}
    {% endfor %}

}  // namespace {{native_namespace}}

#endif  // {{DIR}}_CHAIN_UTILS_H_
