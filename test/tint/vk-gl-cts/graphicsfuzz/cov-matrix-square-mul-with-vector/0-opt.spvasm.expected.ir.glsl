SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2 m0 = mat2(vec2(0.0f), vec2(0.0f));
  mat2 m1 = mat2(vec2(0.0f), vec2(0.0f));
  vec2 v = vec2(0.0f);
  vec2 v_1 = vec2(x_6.x_GLF_uniform_float_values[0].el, -0.540302276611328125f);
  m0 = mat2(v_1, vec2(0.540302276611328125f, x_6.x_GLF_uniform_float_values[0].el));
  m1 = (m0 * m0);
  vec2 v_2 = vec2(x_6.x_GLF_uniform_float_values[0].el);
  v = (v_2 * m1);
  if ((v.x < x_6.x_GLF_uniform_float_values[0].el)) {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[0].el);
  } else {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[1].el);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
