# Copyright 2020 The Tint Authors.
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

# TODO(rharrison): Make a generic fuzzer target template once we have more
#                  fuzzers.

if ({TINT_BUILD_WGSL_READER})
  add_executable(tint_wgsl_reader_fuzzer tint_wgsl_reader_fuzzer.cc)
  target_link_libraries(tint_fuzz libtint)
  tint_default_compile_options(tint_fuzz)
  target_link_options(tint_fuzz PRIVATE
    -fno-omit-frame-pointer
    -fsanitize=fuzzer,address,undefined
    -fsanitize-address-use-after-scope
    -O1
    -g
  )

  target_compile_options(tint_wgsl_reader_fuzzer PRIVATE
    -fsanitize=fuzzer,address,undefined
    -Wno-missing-prototypes
  )
endif()
