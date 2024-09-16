SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  float zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  float x_25 = v.tint_symbol_1.zero;
  float x_28 = v.tint_symbol_1.zero;
  float v_1 = clamp(2.0f, x_25, 1.0f);
  if (any((vec4(v_1, clamp(-1.0f, 0.0f, x_28), 0.0f, 1.0f) != vec4(1.0f, 0.0f, 0.0f, 1.0f)))) {
    x_GLF_color = vec4(0.0f);
  } else {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:24: 'any' : no matching overloaded function found 
ERROR: 0:24: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
