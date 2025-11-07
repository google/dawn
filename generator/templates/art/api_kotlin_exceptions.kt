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
 * Exception thrown when a [GPUDevice] is lost and can no longer be used.
 *
 * @property device The [GPUDevice] that was lost.
 * @property reason The reason code indicating why the device was lost.
 * @param message A human-readable message describing the device loss.
 */
public class DeviceLostException(
  public val device: {{kotlin_name(ns.device)}},
  @{{kotlin_name(ns.device_lost_reason)}} public val reason: Int,
  message: String
) : Exception(message)

{% for value in ns.error.values %}
    /**
     * Exception for {{value.name.CamelCase()}} type errors.
     *
     * @property device The device that encountered the condition.
     */
    public class {{value.name.CamelCase()}}Exception(public val device: {{kotlin_name(ns.device)}}, message: String) : Exception(message);

{% endfor %}
