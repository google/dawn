# Copyright 2026 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# 1. Keep every class that has an `external` method and all its `external` methods.
-keepclasseswithmembers,includedescriptorclasses class androidx.webgpu.** {
    native <methods>;
}

# 2. Keep all constructors, handle field, property getters (val/var), and run()
#    on all classes except Builder and Companion classes.
-if class androidx.webgpu.** {
    native <methods>;
}
-keep class !androidx.webgpu.*$Builder**, !androidx.webgpu.*$Companion, androidx.webgpu.* {
    <init>(...);
    long handle;
    public *** get*();
    public *** is*();
    public void run();
}

-keepclassmembers class androidx.webgpu.GPUOrigin2D, androidx.webgpu.GPUExtent2D, androidx.webgpu.GPUColorSpaceDawn {
    <fields>;
}

# 3. Keep all methods on callback interfaces.
-if class androidx.webgpu.** {
    native <methods>;
}
-keep interface androidx.webgpu.*Callback* {
    <methods>;
}
