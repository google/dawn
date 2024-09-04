SKIP: FAILED

#version 310 es

struct buf2 {
  float zero;
};

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
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf2 x_6;
uniform buf0 x_8;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int x_32 = 0;
  if ((x_6.zero < x_8.x_GLF_uniform_float_values[0].el)) {
    x_32 = x_10.x_GLF_uniform_int_values[1].el;
  } else {
    x_32 = x_10.x_GLF_uniform_int_values[0].el;
  }
  a = ~((x_32 | 1));
  if ((a == ~(x_10.x_GLF_uniform_int_values[0].el))) {
    float v = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[0].el));
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
