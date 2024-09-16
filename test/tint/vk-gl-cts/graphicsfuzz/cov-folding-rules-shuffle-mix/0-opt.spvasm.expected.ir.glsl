SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 threeandfour;
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
  v = vec4(2.0f, 3.0f, 4.0f, 5.0f);
  vec2 v_2 = vec2(1.0f, v_1.tint_symbol_1.threeandfour.y);
  float v_3 = ((bvec2(true, false).x) ? (v_2.x) : (vec2(2.0f, 6.0f).x));
  vec2 v_4 = vec2(v_3, ((bvec2(true, false).y) ? (v_2.y) : (vec2(2.0f, 6.0f).y))).xy;
  v = vec4(v_4, v.zw);
  if (all((v == vec4(1.0f, 6.0f, 4.0f, 5.0f)))) {
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
ERROR: 0:27: 'all' : no matching overloaded function found 
ERROR: 0:27: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
