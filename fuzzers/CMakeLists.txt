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

function(add_tint_fuzzer NAME)
  add_executable(${NAME} ${NAME}.cc)
  target_link_libraries(${NAME} libtint)
  tint_default_compile_options(${NAME})
  target_link_options(${NAME} PRIVATE
    -fno-omit-frame-pointer
    -fsanitize=fuzzer,address,undefined
    -fsanitize-address-use-after-scope
    -O1
    -g
  )

  target_compile_options(${NAME} PRIVATE
    -fsanitize=fuzzer,address,undefined
    -Wno-missing-prototypes
  )
endfunction()

if (${TINT_BUILD_WGSL_READER})
  add_tint_fuzzer(tint_wgsl_reader_fuzzer)
endif()

if (${TINT_BUILD_SPV_READER})
  add_tint_fuzzer(tint_spv_reader_fuzzer)
endif()
