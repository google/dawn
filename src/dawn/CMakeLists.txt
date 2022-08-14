# Copyright 2020 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###############################################################################
# Dawn projects
###############################################################################

add_subdirectory(common)
add_subdirectory(platform)
add_subdirectory(native)
add_subdirectory(wire)
# TODO(dawn:269): Remove once the implementation-based swapchains are removed.
add_subdirectory(utils)
add_subdirectory(glfw)

if (DAWN_BUILD_SAMPLES)
    #TODO(dawn:269): Add this once implementation-based swapchains are removed.
    #add_subdirectory(src/utils)
    add_subdirectory(samples)
endif()

if (DAWN_BUILD_NODE_BINDINGS)
    set(NODE_BINDING_DEPS
        ${NODE_ADDON_API_DIR}
        ${NODE_API_HEADERS_DIR}
        ${WEBGPU_IDL_PATH}
    )
    foreach(DEP ${NODE_BINDING_DEPS})
        if (NOT EXISTS ${DEP})
            message(FATAL_ERROR
                "DAWN_BUILD_NODE_BINDINGS requires missing dependency '${DEP}'\n"
                "Please follow the 'Fetch dependencies' instructions at:\n"
                "./src/dawn/node/README.md"
            )
        endif()
    endforeach()
    if (NOT CMAKE_POSITION_INDEPENDENT_CODE)
        message(FATAL_ERROR "DAWN_BUILD_NODE_BINDINGS requires building with DAWN_ENABLE_PIC")
    endif()

    add_subdirectory(node)
endif()

###############################################################################
# Dawn headers
###############################################################################

DawnJSONGenerator(
    TARGET "headers"
    PRINT_NAME "Dawn headers"
    RESULT_VARIABLE "DAWN_HEADERS_GEN_SOURCES"
)

# Headers only INTERFACE library with generated headers don't work in CMake
# because the GENERATED property is local to a directory. Instead we make a
# STATIC library with a placeholder cpp file.
#
# INTERFACE libraries can only have INTERFACE sources so the sources get added
# to the dependant's list of sources. If these dependents are in another
# directory, they don't see the GENERATED property and fail to configure
# because the file doesn't exist on disk.
add_library(dawn_headers STATIC ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_headers)
target_sources(dawn_headers PRIVATE
    "${DAWN_INCLUDE_DIR}/dawn/dawn_wsi.h"
    ${DAWN_HEADERS_GEN_SOURCES}
)
target_link_libraries(dawn_headers INTERFACE dawn_public_config)

###############################################################################
# Dawn C++ headers
###############################################################################

DawnJSONGenerator(
    TARGET "cpp_headers"
    PRINT_NAME "Dawn C++ headers"
    RESULT_VARIABLE "DAWNCPP_HEADERS_GEN_SOURCES"
)

# This headers only library needs to be a STATIC library, see comment for
# dawn_headers above.
add_library(dawncpp_headers STATIC ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawncpp_headers)
target_sources(dawncpp_headers PRIVATE
    "${DAWN_INCLUDE_DIR}/dawn/EnumClassBitmasks.h"
    ${DAWNCPP_HEADERS_GEN_SOURCES}
)
target_link_libraries(dawncpp_headers INTERFACE dawn_headers)

###############################################################################
# Dawn C++ wrapper
###############################################################################

DawnJSONGenerator(
    TARGET "cpp"
    PRINT_NAME "Dawn C++ wrapper"
    RESULT_VARIABLE "DAWNCPP_GEN_SOURCES"
)

add_library(dawncpp STATIC ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawncpp)
target_sources(dawncpp PRIVATE ${DAWNCPP_GEN_SOURCES})
target_link_libraries(dawncpp PUBLIC dawncpp_headers)

add_library(webgpu_cpp ALIAS dawncpp)

###############################################################################
# libdawn_proc
###############################################################################

DawnJSONGenerator(
    TARGET "proc"
    PRINT_NAME "Dawn C++ wrapper"
    RESULT_VARIABLE "DAWNPROC_GEN_SOURCES"
)

add_library(dawn_proc ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_proc)
target_compile_definitions(dawn_proc PRIVATE "WGPU_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_proc PRIVATE "WGPU_SHARED_LIBRARY")
endif()
target_sources(dawn_proc PRIVATE ${DAWNPROC_GEN_SOURCES})
target_link_libraries(dawn_proc PUBLIC dawn_headers)

###############################################################################
# Other generated files (upstream header, emscripten header, emscripten bits)
###############################################################################

DawnJSONGenerator(
    TARGET "webgpu_headers"
    PRINT_NAME "WebGPU headers"
    RESULT_VARIABLE "WEBGPU_HEADERS_GEN_SOURCES"
)
add_custom_target(webgpu_headers_gen
    DEPENDS ${WEBGPU_HEADERS_GEN_SOURCES}
)

DawnJSONGenerator(
    TARGET "emscripten_bits"
    PRINT_NAME "Emscripten WebGPU bits"
    RESULT_VARIABLE "EMSCRIPTEN_BITS_GEN_SOURCES"
)
add_custom_target(emscripten_bits_gen
    DEPENDS ${EMSCRIPTEN_BITS_GEN_SOURCES}
)
