#version 310 es
precision highp float;
precision highp int;

layout(binding = 2, std430)
buffer f_a_block_ssbo {
  int inner;
} v;
bool continue_execution = true;
uniform highp sampler2D f_t_s;
layout(location = 0) in float tint_interstage_location0;
layout(location = 1) in vec2 tint_interstage_location1;
layout(location = 0) out int foo_loc0_Output;
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
int foo_inner(float v_1, vec2 coord) {
  if ((v_1 == 0.0f)) {
    continue_execution = false;
  }
  int result = tint_f32_to_i32(texture(f_t_s, coord).x);
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((i < 10)) {
      } else {
        break;
      }
      int v_2 = i;
      uint v_3 = uint(result);
      result = int((v_3 + uint(v_2)));
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        int v_4 = 0;
        if (continue_execution) {
          v_4 = atomicAdd(v.inner, 1);
        }
        i = v_4;
      }
      continue;
    }
  }
  if (!(continue_execution)) {
    discard;
  }
  return result;
}
void main() {
  foo_loc0_Output = foo_inner(tint_interstage_location0, tint_interstage_location1);
}
