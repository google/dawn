SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct buf2 {
  vec2 resolution;
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf2 x_8;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_10;
void main_1() {
  float f = 0.0f;
  float v = (x_6.x_GLF_uniform_float_values[1].el * x_8.resolution.x);
  float v_1 = x_6.x_GLF_uniform_float_values[2].el;
  float v_2 = (v + (v_1 * round(x_8.resolution.x)));
  f = (v_2 + x_8.resolution.y);
  if ((f == x_6.x_GLF_uniform_float_values[0].el)) {
    float v_3 = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_4 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_5 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_10.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[1].el));
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
