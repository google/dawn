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
  vec4 N = vec4(0.0f);
  vec4 I = vec4(0.0f);
  vec4 Nref = vec4(0.0f);
  vec4 v = vec4(0.0f);
  N = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  I = vec4(4.0f, 87.589996337890625f, v_1.tint_symbol_1.quarter, 92.51000213623046875f);
  Nref = vec4(17.049999237060546875f, -6.09999990463256835938f, 4329.37060546875f, 2.70000004768371582031f);
  v = faceforward(N, I, Nref);
  if (all((v == vec4(-1.0f, -2.0f, -3.0f, -4.0f)))) {
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
ERROR: 0:29: 'all' : no matching overloaded function found 
ERROR: 0:29: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
