// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/reader/spirv/construct.h"

namespace tint::reader::spirv {

Construct::Construct(const Construct* the_parent,
                     int the_depth,
                     Kind the_kind,
                     uint32_t the_begin_id,
                     uint32_t the_end_id,
                     uint32_t the_begin_pos,
                     uint32_t the_end_pos,
                     uint32_t the_scope_end_pos)
    : parent(the_parent),
      enclosing_loop(
          // Compute the enclosing loop construct. Doing this in the
          // constructor member list lets us make the member const.
          // Compare parent depth because loop and continue are siblings and
          // it's incidental which will appear on the stack first.
          the_kind == kLoop
              ? this
              : ((parent && parent->depth < the_depth) ? parent->enclosing_loop : nullptr)),
      enclosing_continue(
          // Compute the enclosing continue construct. Doing this in the
          // constructor member list lets us make the member const.
          // Compare parent depth because loop and continue are siblings and
          // it's incidental which will appear on the stack first.
          the_kind == kContinue
              ? this
              : ((parent && parent->depth < the_depth) ? parent->enclosing_continue : nullptr)),
      enclosing_loop_or_continue_or_switch(
          // Compute the enclosing loop or continue or switch construct.
          // Doing this in the constructor member list lets us make the
          // member const.
          // Compare parent depth because loop and continue are siblings and
          // it's incidental which will appear on the stack first.
          (the_kind == kLoop || the_kind == kContinue || the_kind == kSwitchSelection)
              ? this
              : ((parent && parent->depth < the_depth)
                     ? parent->enclosing_loop_or_continue_or_switch
                     : nullptr)),
      depth(the_depth),
      kind(the_kind),
      begin_id(the_begin_id),
      end_id(the_end_id),
      begin_pos(the_begin_pos),
      end_pos(the_end_pos),
      scope_end_pos(the_scope_end_pos) {}

}  // namespace tint::reader::spirv
