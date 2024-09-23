SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  int zero;
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
  vec2 a = vec2(0.0f);
  vec2 b = vec2(0.0f);
  a = vec2(1.0f);
  int x_38 = v.tint_symbol_1.zero;
  if ((x_38 == 1)) {
    float x_43 = a.x;
    a[0u] = (x_43 + 1.0f);
  }
  float x_47 = a.y;
  b = (vec2(x_47, x_47) + vec2(2.0f, 3.0f));
  vec2 x_50 = b;
  if (all((x_50 == vec2(3.0f, 4.0f)))) {
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
ERROR: 0:32: 'all' : no matching overloaded function found 
ERROR: 0:32: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
