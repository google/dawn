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
  float x_35 = x_6.x_GLF_uniform_float_values[0].el;
  float x_37 = x_6.x_GLF_uniform_float_values[0].el;
  vec2 v_1 = vec2(x_35, -0.540302276611328125f);
  m0 = mat2(v_1, vec2(0.540302276611328125f, x_37));
  mat2 x_41 = m0;
  mat2 x_42 = m0;
  m1 = (x_41 * x_42);
  float x_45 = x_6.x_GLF_uniform_float_values[0].el;
  mat2 x_47 = m1;
  v = (vec2(x_45, x_45) * x_47);
  float x_50 = v.x;
  float x_52 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_50 < x_52)) {
    float x_58 = x_6.x_GLF_uniform_float_values[0].el;
    float x_60 = x_6.x_GLF_uniform_float_values[1].el;
    float x_62 = x_6.x_GLF_uniform_float_values[1].el;
    float x_64 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_58, x_60, x_62, x_64);
  } else {
    float x_67 = x_6.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4(x_67, x_67, x_67, x_67);
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
