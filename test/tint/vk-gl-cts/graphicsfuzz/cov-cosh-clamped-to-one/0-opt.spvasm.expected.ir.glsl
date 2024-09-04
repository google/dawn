SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_8;
void main_1() {
  vec4 v = vec4(0.0f);
  v = clamp(vec4(1.54308068752288818359f), vec4(x_6.x_GLF_uniform_float_values[0].el), vec4(1.0f));
  float v_1 = v.x;
  float v_2 = float(x_8.x_GLF_uniform_int_values[0].el);
  float v_3 = float(x_8.x_GLF_uniform_int_values[0].el);
  x_GLF_color = vec4(v_1, v_2, v_3, v.z);
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
