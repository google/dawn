//* Copyright 2019 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DAWNWIRE_SERVER_SERVERBASE_H_
#define DAWNWIRE_SERVER_SERVERBASE_H_

#include "dawn/dawn_proc_table.h"
#include "dawn/wire/ChunkedCommandHandler.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireDeserializeAllocator.h"
#include "dawn/wire/server/ObjectStorage.h"

namespace dawn::wire::server {

    class ServerBase : public ChunkedCommandHandler, public ObjectIdResolver {
      public:
        ServerBase() = default;
        ~ServerBase() override = default;

      protected:
        void DestroyAllObjects(const DawnProcTable& procs) {
            //* Release devices first to force completion of any async work.
            {
                std::vector<WGPUDevice> handles = mKnownDevice.AcquireAllHandles();
                for (WGPUDevice handle : handles) {
                    procs.deviceRelease(handle);
                }
            }
            //* Free all objects when the server is destroyed
            {% for type in by_category["object"] if type.name.get() != "device" %}
                {
                    std::vector<{{as_cType(type.name)}}> handles = mKnown{{type.name.CamelCase()}}.AcquireAllHandles();
                    for ({{as_cType(type.name)}} handle : handles) {
                        procs.{{as_varName(type.name, Name("release"))}}(handle);
                    }
                }
            {% endfor %}
        }

        {% for type in by_category["object"] %}
            const KnownObjects<{{as_cType(type.name)}}>& {{type.name.CamelCase()}}Objects() const {
                return mKnown{{type.name.CamelCase()}};
            }
            KnownObjects<{{as_cType(type.name)}}>& {{type.name.CamelCase()}}Objects() {
                return mKnown{{type.name.CamelCase()}};
            }
        {% endfor %}

        {% for type in by_category["object"] if type.name.CamelCase() in server_reverse_lookup_objects %}
            const ObjectIdLookupTable<{{as_cType(type.name)}}>& {{type.name.CamelCase()}}ObjectIdTable() const {
                return m{{type.name.CamelCase()}}IdTable;
            }
            ObjectIdLookupTable<{{as_cType(type.name)}}>& {{type.name.CamelCase()}}ObjectIdTable() {
                return m{{type.name.CamelCase()}}IdTable;
            }
        {% endfor %}

      private:
        // Implementation of the ObjectIdResolver interface
        {% for type in by_category["object"] %}
            WireResult GetFromId(ObjectId id, {{as_cType(type.name)}}* out) const final {
                return mKnown{{type.name.CamelCase()}}.GetNativeHandle(id, out);
            }

            WireResult GetOptionalFromId(ObjectId id, {{as_cType(type.name)}}* out) const final {
                if (id == 0) {
                    *out = nullptr;
                    return WireResult::Success;
                }

                return GetFromId(id, out);
            }
        {% endfor %}

        //* The list of known IDs for each object type.
        {% for type in by_category["object"] %}
            KnownObjects<{{as_cType(type.name)}}> mKnown{{type.name.CamelCase()}};
        {% endfor %}

        {% for type in by_category["object"] if type.name.CamelCase() in server_reverse_lookup_objects %}
            ObjectIdLookupTable<{{as_cType(type.name)}}> m{{type.name.CamelCase()}}IdTable;
        {% endfor %}
    };

}  // namespace dawn::wire::server

#endif  // DAWNWIRE_SERVER_SERVERBASE_H_
