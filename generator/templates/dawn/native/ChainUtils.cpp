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

//
// Unpacked chain helpers.
//
{% for type in by_category["structure"] %}
    {% if type.extensible == "in" %}
        {% set unpackedChain = "Unpacked" + as_cppType(type.name) + "Chain" %}
        ResultOrError<{{unpackedChain}}> ValidateAndUnpackChain(const {{as_cppType(type.name)}}* chain) {
            const ChainedStruct* next = chain->nextInChain;
            {{unpackedChain}} result;

            //* Branching generation block to avoid warnings when the struct is not currently
            //* extendable:
            //*   -Wunreachable-code-loop-increment:
            //*      error: loop will run at most once (loop increment never executed)
            {% if len(type.extensions) == 0 %}
                if (next != nullptr) {
                    return DAWN_VALIDATION_ERROR(
                        "Unexpected chained struct of type %s found on %s chain.",
                        next->sType, "{{as_cppType(type.name)}}"
                    );
                }
            {% else %}
                for (; next != nullptr; next = next->nextInChain) {
                    switch (next->sType) {
                        {% for extension in type.extensions %}
                            case STypeFor<{{as_cppType(extension.name)}}>: {
                                auto& member = std::get<const {{as_cppType(extension.name)}}*>(result);
                                if (member != nullptr) {
                                    return DAWN_VALIDATION_ERROR(
                                        "Duplicate chained struct of type %s found on %s chain.",
                                        next->sType, "{{as_cppType(type.name)}}"
                                    );
                                } else {
                                    member = static_cast<const {{as_cppType(extension.name)}}*>(next);
                                }
                                break;
                            }
                        {% endfor %}
                        default:
                          return DAWN_VALIDATION_ERROR(
                              "Unexpected chained struct of type %s found on %s chain.",
                              next->sType, "{{as_cppType(type.name)}}"
                          );
                    }
                }
            {% endif %}
            return result;
        }

    {% endif %}
{% endfor %}

}  // namespace {{native_namespace}}
