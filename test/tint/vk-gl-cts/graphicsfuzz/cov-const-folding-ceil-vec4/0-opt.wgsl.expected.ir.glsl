SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  float quarter;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec4 v = vec4(0.0f);
  float x_32 = v_1.tint_symbol_1.quarter;
  v = ceil(vec4(424.113006591796875f, x_32, 1.29999995231628417969f, 19.6200008392333984375f));
  vec4 x_35 = v;
  if (all((x_35 == vec4(425.0f, 1.0f, 2.0f, 20.0f)))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
ERROR: 0:25: 'all' : no matching overloaded function found 
ERROR: 0:25: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
