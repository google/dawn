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

// Command handlers & doers
{% for command in cmd_records["return command"] %}
    {% set Suffix = command.name.CamelCase() %}
    {% set CmdName = "Return" + Suffix + "Cmd" %}
    {% set spanify = CmdName not in cmd_spanification_blocklist %}
    WireResult Handle{{Suffix}}(DeserializeBuffer* deserializeBuffer);
    WireResult Do{{Suffix}}(
        {%- for member in command.members if (not spanify or not member.is_length) -%}
            {%- if not loop.first -%}, {% endif %}
            {%- if member.handle_type -%}
                {{as_wireType(member.handle_type)}} {{as_varName(member.name)}}
            {%- elif spanify and member.length and member.length != "constant" -%}
                {% set element_type = "std::remove_pointer_t<" + decorate(as_cType(member.type.name, spanify), member) + ">" %}
                {% set index_type = member.length.type.name.canonical_case() %}
                ityp::span<{{index_type}}, {{element_type}}> {{as_varName(member.name)}}
            {%- else -%}
                {{as_annotated_wireType(member)}}
            {%- endif -%}
        {%- endfor -%}
    );

{% endfor %}
