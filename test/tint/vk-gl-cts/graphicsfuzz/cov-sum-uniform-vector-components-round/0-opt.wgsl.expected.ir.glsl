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
  float x_37 = x_6.x_GLF_uniform_float_values[1].el;
  float x_39 = x_8.resolution.x;
  float x_42 = x_6.x_GLF_uniform_float_values[2].el;
  float x_44 = x_8.resolution.x;
  float x_49 = x_8.resolution.y;
  f = (((x_37 * x_39) + (x_42 * round(x_44))) + x_49);
  float x_51 = f;
  float x_53 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_51 == x_53)) {
    int x_59 = x_10.x_GLF_uniform_int_values[0].el;
    int x_62 = x_10.x_GLF_uniform_int_values[1].el;
    int x_65 = x_10.x_GLF_uniform_int_values[1].el;
    int x_68 = x_10.x_GLF_uniform_int_values[0].el;
    float v = float(x_59);
    float v_1 = float(x_62);
    float v_2 = float(x_65);
    x_GLF_color = vec4(v, v_1, v_2, float(x_68));
  } else {
    int x_72 = x_10.x_GLF_uniform_int_values[1].el;
    float x_73 = float(x_72);
    x_GLF_color = vec4(x_73, x_73, x_73, x_73);
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
