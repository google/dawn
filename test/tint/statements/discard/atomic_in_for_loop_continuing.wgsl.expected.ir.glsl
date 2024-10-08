#version 310 es
precision highp float;
precision highp int;

layout(binding = 2, std430)
buffer tint_symbol_2_1_ssbo {
  int tint_symbol_1;
} v;
bool continue_execution = true;
uniform highp sampler2D t_s;
layout(location = 0) in float foo_loc0_Input;
layout(location = 1) in vec2 foo_loc1_Input;
layout(location = 0) out int foo_loc0_Output;
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
int foo_inner(float tint_symbol, vec2 coord) {
  if ((tint_symbol == 0.0f)) {
    continue_execution = false;
  }
  int result = tint_f32_to_i32(texture(t_s, coord)[0u]);
  {
    int i = 0;
    while(true) {
      if ((i < 10)) {
      } else {
        break;
      }
      result = (result + i);
      {
        int v_1 = 0;
        if (continue_execution) {
          v_1 = atomicAdd(v.tint_symbol_1, 1);
        }
        i = v_1;
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
  foo_loc0_Output = foo_inner(foo_loc0_Input, foo_loc1_Input);
}
