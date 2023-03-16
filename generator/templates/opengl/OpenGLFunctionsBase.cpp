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

#include "dawn/native/opengl/OpenGLFunctionsBase_autogen.h"

namespace dawn::native::opengl {

template<typename T>
MaybeError OpenGLFunctionsBase::LoadProc(GetProcAddress getProc, T* memberProc, const char* name) {
    *memberProc = reinterpret_cast<T>(getProc(name));
    if (DAWN_UNLIKELY(memberProc == nullptr)) {
        return DAWN_INTERNAL_ERROR(std::string("Couldn't load GL proc: ") + name);
    }
    return {};
}

MaybeError OpenGLFunctionsBase::LoadOpenGLESProcs(GetProcAddress getProc, int majorVersion, int minorVersion) {
    {% for block in gles_blocks %}
        // OpenGL ES {{block.version.major}}.{{block.version.minor}}
        if (majorVersion > {{block.version.major}} || (majorVersion == {{block.version.major}} && minorVersion >= {{block.version.minor}})) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
            {% endfor %}
        }

    {% endfor %}

    InitializeSupportedGLExtensions();

    {% for block in extension_gles_blocks %}
        // {{block.extension}}
        if (IsGLExtensionSupported("{{block.extension}}")) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
        {% endfor %}
        }
    {% endfor %}

    // GL_ANGLE_base_vertex_base_instance
    // See crbug.com/dawn/1715 for why this is embedded
    if (IsGLExtensionSupported("GL_ANGLE_base_vertex_base_instance")) {
        DAWN_TRY(LoadProc(getProc, &DrawArraysInstancedBaseInstanceANGLE, "glDrawArraysInstancedBaseInstanceANGLE"));
        DAWN_TRY(LoadProc(getProc, &DrawElementsInstancedBaseVertexBaseInstanceANGLE, "glDrawElementsInstancedBaseVertexBaseInstanceANGLE"));
        DAWN_TRY(LoadProc(getProc, &MultiDrawArraysInstancedBaseInstanceANGLE, "glMultiDrawArraysInstancedBaseInstanceANGLE"));
        DAWN_TRY(LoadProc(getProc, &MultiDrawElementsInstancedBaseVertexBaseInstanceANGLE, "glMultiDrawElementsInstancedBaseVertexBaseInstanceANGLE"));
    }

    return {};
}

MaybeError OpenGLFunctionsBase::LoadDesktopGLProcs(GetProcAddress getProc, int majorVersion, int minorVersion) {
    {% for block in desktop_gl_blocks %}
        // Desktop OpenGL {{block.version.major}}.{{block.version.minor}}
        if (majorVersion > {{block.version.major}} || (majorVersion == {{block.version.major}} && minorVersion >= {{block.version.minor}})) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
            {% endfor %}
        }

    {% endfor %}

    InitializeSupportedGLExtensions();

    {% for block in extension_desktop_gl_blocks %}
        // {{block.extension}}
        if (IsGLExtensionSupported("{{block.extension}}")) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
            {% endfor %}
        }
    {% endfor %}

    return {};
}

void OpenGLFunctionsBase::InitializeSupportedGLExtensions() {
    int32_t numExtensions;
    GetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (int32_t i = 0; i < numExtensions; ++i) {
        const char* extensionName = reinterpret_cast<const char*>(GetStringi(GL_EXTENSIONS, i));
        mSupportedGLExtensionsSet.insert(extensionName);
    }
}

bool OpenGLFunctionsBase::IsGLExtensionSupported(const char* extension) const {
    ASSERT(extension != nullptr);
    return mSupportedGLExtensionsSet.count(extension) != 0;
}

}  // namespace dawn::native::opengl
