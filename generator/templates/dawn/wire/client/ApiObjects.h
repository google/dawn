//* Copyright 2019 The Dawn Authors
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

#ifndef DAWNWIRE_CLIENT_APIOBJECTS_AUTOGEN_H_
#define DAWNWIRE_CLIENT_APIOBJECTS_AUTOGEN_H_

#include "dawn/wire/ObjectType_autogen.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

    template<typename T>
    inline constexpr ObjectType ObjectTypeToTypeEnum = static_cast<ObjectType>(-1);

    {% for type in by_category["object"] %}
        {% set Type = type.name.CamelCase() %}
        {% if type.name.CamelCase() in client_special_objects %}
            class {{Type}};
        {% else %}
            struct {{type.name.CamelCase()}} final : ObjectBase {
                using ObjectBase::ObjectBase;
            };
        {% endif %}

        inline {{Type}}* FromAPI(WGPU{{Type}} obj) {
            return reinterpret_cast<{{Type}}*>(obj);
        }
        inline WGPU{{Type}} ToAPI({{Type}}* obj) {
            return reinterpret_cast<WGPU{{Type}}>(obj);
        }

        template <>
        inline constexpr ObjectType ObjectTypeToTypeEnum<{{Type}}> = ObjectType::{{Type}};

    {% endfor %}
}  // namespace dawn::wire::client

#endif  // DAWNWIRE_CLIENT_APIOBJECTS_AUTOGEN_H_
