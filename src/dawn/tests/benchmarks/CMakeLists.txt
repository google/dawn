# Copyright 2023 The Dawn Authors
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

if (${DAWN_BUILD_BENCHMARKS})
  add_executable(dawn_benchmarks
    "NullDeviceSetup.cpp"
    "NullDeviceSetup.h"
    "ObjectCreation.cpp"
  )
  set_target_properties(dawn_benchmarks PROPERTIES FOLDER "Benchmarks")

  target_include_directories(dawn_benchmarks PUBLIC "${PROJECT_SOURCE_DIR}/include")
  target_include_directories(dawn_benchmarks PUBLIC "${PROJECT_SOURCE_DIR}/src")

  target_link_libraries(dawn_benchmarks PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    dawn_common
    dawn_native
    dawncpp_headers
    dawncpp
    dawn_proc)
endif()
