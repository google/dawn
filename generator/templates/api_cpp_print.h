//* Copyright 2021 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.

{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
#ifndef {{API}}_CPP_PRINT_H_
#define {{API}}_CPP_PRINT_H_

#include "dawn/{{api}}_cpp.h"

#include <iomanip>
#include <ios>
#include <ostream>
#include <type_traits>

namespace {{metadata.namespace}} {

  {% for type in by_category["enum"] %}
      template <typename CharT, typename Traits>
      std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& o, {{as_cppType(type.name)}} value) {
          switch (value) {
            {% for value in type.values %}
              case {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}:
                o << "{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}";
                break;
            {% endfor %}
              default:
                o << "{{as_cppType(type.name)}}::" << std::showbase << std::hex << std::setfill('0') << std::setw(4) << static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value);
          }
          return o;
      }
  {% endfor %}

  {% for type in by_category["bitmask"] %}
      template <typename CharT, typename Traits>
      std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& o, {{as_cppType(type.name)}} value) {
        o << "{{as_cppType(type.name)}}::";
        if (!static_cast<bool>(value)) {
          {% for value in type.values if value.value == 0 %}
            // 0 is often explicitly declared as None.
            o << "{{as_cppEnum(value.name)}}";
          {% else %}
            o << std::showbase << std::hex << std::setfill('0') << std::setw(4) << 0;
          {% endfor %}
          return o;
        }

        bool moreThanOneBit = !HasZeroOrOneBits(value);
        if (moreThanOneBit) {
          o << "(";
        }

        bool first = true;
        {% for value in type.values if value.value != 0 %}
          if (value & {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}) {
            if (!first) {
              o << "|";
            }
            first = false;
            o << "{{as_cppEnum(value.name)}}";
            value &= ~{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}};
          }
        {% endfor %}

        if (static_cast<bool>(value)) {
          if (!first) {
            o << "|";
          }
          o << std::showbase << std::hex << std::setfill('0') << std::setw(4) << static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value);
        }

        if (moreThanOneBit) {
          o << ")";
        }
        return o;
      }
  {% endfor %}

}  // namespace {{metadata.namespace}}

#endif // {{API}}_CPP_PRINT_H_
