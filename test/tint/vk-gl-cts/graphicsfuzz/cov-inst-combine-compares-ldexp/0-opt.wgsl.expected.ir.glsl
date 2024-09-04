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
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_7;
void main_1() {
  float x_29 = x_5.x_GLF_uniform_float_values[0].el;
  float x_32 = x_5.x_GLF_uniform_float_values[0].el;
  if ((ldexp(x_29, 100) == x_32)) {
    int x_38 = x_7.x_GLF_uniform_int_values[1].el;
    int x_41 = x_7.x_GLF_uniform_int_values[0].el;
    int x_44 = x_7.x_GLF_uniform_int_values[0].el;
    int x_47 = x_7.x_GLF_uniform_int_values[1].el;
    float v = float(x_38);
    float v_1 = float(x_41);
    float v_2 = float(x_44);
    x_GLF_color = vec4(v, v_1, v_2, float(x_47));
  } else {
    int x_51 = x_7.x_GLF_uniform_int_values[1].el;
    int x_54 = x_7.x_GLF_uniform_int_values[0].el;
    int x_57 = x_7.x_GLF_uniform_int_values[0].el;
    int x_60 = x_7.x_GLF_uniform_int_values[1].el;
    float v_3 = float(x_51);
    float v_4 = float(x_54);
    float v_5 = float(x_57);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_60));
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
