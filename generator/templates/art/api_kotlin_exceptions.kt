//* Copyright 2025 The Dawn & Tint Authors
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
@file:JvmName("Exceptions")

package {{ kotlin_package }}
{% set ns = namespace() %}
{% for enum in by_category["enum"] %}
    {% if enum.name.get() == "error type" %}{% set ns.error = enum %}{% endif %}
    {% if enum.name.get() == "device lost reason" %}{% set ns.device_lost_reason = enum %}{% endif %}
{% endfor %}
{% for obj in by_category["object"] if obj.name.get() == "device" %}
    {% set ns.device = obj %}
{% endfor %}

/**
 * Exception for errors originating from the Dawn WebGPU library that do not fit
 * into more specific WebGPU error categories.
 *
 * @param message A detailed message explaining the error.
 */
public class DawnException(message: String) : Exception(message)

/**
 * Exception thrown when a [{{kotlin_name(ns.device)}}] is lost and can no longer be used.
 *
 * @param device The [{{kotlin_name(ns.device)}}] that was lost.
 * @param reason The reason code indicating why the device was lost.
 * @param message A human-readable message describing the device loss.
 */
public class DeviceLostException(
  public val device: {{kotlin_name(ns.device)}},
  @{{kotlin_name(ns.device_lost_reason)}} public val reason: Int,
  message: String
) : Exception(message)

/**
 * Base class for exceptions that can happen at runtime.
 */
public open class WebGpuRuntimeException(message: String): Exception(message)

{% for value in ns.error.values if value.name.get() != "no error" %}
    /**
     * Exception for {{value.name.CamelCase()}} type errors.
     *
     * @param message A message explaining the error.
     */
    public class {{value.name.CamelCase()}}Exception(message: String) : WebGpuRuntimeException(message);

{% endfor %}

/**
 * Create the exception for the appropriate error type.
 * @param type The [{{ kotlin_name(ns.error) }}].
 * @param message A human-readable message describing the device loss.
 */
public fun getException(@{{ kotlin_name(ns.error) }} type: Int, message: String): WebGpuRuntimeException =
    when (type) {
        {% for value in ns.error.values if value.name.get() != "no error" %}
            {{ kotlin_name(ns.error) }}.{{value.name.CamelCase()}} -> {{value.name.CamelCase()}}Exception(message)
        {% endfor %}
        else -> UnknownException(message)
    }

//* Generate a custom exception for every enum ending 'status'.
//* 'status' is renamed 'web gpu status'.
{% for enum in by_category['enum'] if include_enum(enum) and enum.name.chunks[-1] == 'status' %}
    {% set success = enum.values[0] %}  //* 'Success' is conventionally the first enum.
    {% set exception_name = (enum.name.chunks[:-1] if len(enum.name.chunks) > 1 else ['web', 'gpu']) | map('title') | join + 'Exception' %}
    public class {{ exception_name }} (
        public val reason: String = "",
        @{{ enum.name.CamelCase() }} public val status: Int = {{ enum.name.CamelCase() }}.{{ success.name.CamelCase() }}) : Exception(
            (if (status != {{ enum.name.CamelCase() }}.{{ success.name.CamelCase() }}) "${ {{ enum.name.CamelCase() }}.toString(status)}: " else "") + reason) {
    }

{% endfor %}
