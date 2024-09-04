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
  float x_34 = x_6.zero;
  float x_36 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_34 < x_36)) {
    int x_42 = x_10.x_GLF_uniform_int_values[1].el;
    x_32 = x_42;
  } else {
    int x_44 = x_10.x_GLF_uniform_int_values[0].el;
    x_32 = x_44;
  }
  int x_45 = x_32;
  a = ~((x_45 | 1));
  int x_48 = a;
  int x_50 = x_10.x_GLF_uniform_int_values[0].el;
  if ((x_48 == ~(x_50))) {
    int x_57 = x_10.x_GLF_uniform_int_values[0].el;
    int x_60 = x_10.x_GLF_uniform_int_values[1].el;
    int x_63 = x_10.x_GLF_uniform_int_values[1].el;
    int x_66 = x_10.x_GLF_uniform_int_values[0].el;
    float v = float(x_57);
    float v_1 = float(x_60);
    float v_2 = float(x_63);
    x_GLF_color = vec4(v, v_1, v_2, float(x_66));
  } else {
    int x_70 = x_10.x_GLF_uniform_int_values[1].el;
    float x_71 = float(x_70);
    x_GLF_color = vec4(x_71, x_71, x_71, x_71);
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
