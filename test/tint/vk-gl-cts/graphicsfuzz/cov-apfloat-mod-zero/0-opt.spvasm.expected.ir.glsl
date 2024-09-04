SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float undefined = 0.0f;
  bool x_51 = false;
  bool x_52 = false;
  undefined = 0.0f;
  bool x_44 = (x_6.x_GLF_uniform_int_values[0].el == (x_6.x_GLF_uniform_int_values[0].el + x_6.x_GLF_uniform_int_values[1].el));
  x_52 = x_44;
  if (!(x_44)) {
    x_51 = (undefined > x_8.x_GLF_uniform_float_values[0].el);
    x_52 = x_51;
  }
  if (x_52) {
    float v = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
