// Copyright 2021 The Dawn Authors
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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
#ifndef {{DIR}}_CHAIN_UTILS_H_
#define {{DIR}}_CHAIN_UTILS_H_

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set prefix = metadata.proc_table_prefix.lower() %}
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/Error.h"

namespace {{native_namespace}} {
    {% for value in types["s type"].values %}
        {% if value.valid %}
            {% set const_qualifier = "const " if types[value.name.get()].chained == "in" else "" %}
            {% set chained_struct_type = "ChainedStruct" if types[value.name.get()].chained == "in" else "ChainedStructOut" %}
            void FindInChain({{const_qualifier}}{{chained_struct_type}}* chain, {{const_qualifier}}{{as_cppEnum(value.name)}}** out);
        {% endif %}
    {% endfor %}

    // Verifies that |chain| only contains ChainedStructs of types enumerated in
    // |oneOfConstraints| and contains no duplicate sTypes. Each vector in
    // |oneOfConstraints| defines a set of sTypes that cannot coexist in the same chain.
    // For example:
    //   ValidateSTypes(chain, { { ShaderModuleSPIRVDescriptor, ShaderModuleWGSLDescriptor } }))
    //   ValidateSTypes(chain, { { Extension1 }, { Extension2 } })
    {% set namespace = metadata.namespace %}
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

}  // namespace {{native_namespace}}

#endif  // {{DIR}}_CHAIN_UTILS_H_
