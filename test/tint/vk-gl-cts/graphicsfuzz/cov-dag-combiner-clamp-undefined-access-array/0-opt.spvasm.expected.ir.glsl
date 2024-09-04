SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float A1[3] = float[3](0.0f, 0.0f, 0.0f);
  int a = 0;
  float b = 0.0f;
  bool c = false;
  bool x_36 = false;
  A1 = float[3](x_6.x_GLF_uniform_float_values[2].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el);
  int v = x_9.x_GLF_uniform_int_values[1].el;
  a = min(max(x_9.x_GLF_uniform_int_values[0].el, x_9.x_GLF_uniform_int_values[0].el), v);
  int v_1 = x_9.x_GLF_uniform_int_values[0].el;
  b = A1[min(max(a, x_9.x_GLF_uniform_int_values[1].el), v_1)];
  if ((b < A1[x_9.x_GLF_uniform_int_values[1].el])) {
    x_36 = (x_6.x_GLF_uniform_float_values[0].el > x_6.x_GLF_uniform_float_values[2].el);
  } else {
    x_36 = (x_6.x_GLF_uniform_float_values[0].el < A1[x_9.x_GLF_uniform_int_values[2].el]);
  }
  c = x_36;
  if (c) {
    float v_2 = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_3 = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_4 = float(x_9.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_9.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el);
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
