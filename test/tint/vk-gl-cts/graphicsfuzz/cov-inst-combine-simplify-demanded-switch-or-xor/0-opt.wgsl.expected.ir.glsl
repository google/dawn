SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
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


uniform buf0 x_6;
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 color = vec4(0.0f);
  float x_29 = x_6.x_GLF_uniform_float_values[0].el;
  float x_31 = x_6.x_GLF_uniform_float_values[0].el;
  float x_33 = x_6.x_GLF_uniform_float_values[0].el;
  float x_35 = x_6.x_GLF_uniform_float_values[1].el;
  color = vec4(x_29, x_31, x_33, x_35);
  int x_38 = x_8.x_GLF_uniform_int_values[1].el;
  switch(((1 | x_38) ^ 1)) {
    case 0:
    {
      int x_44 = x_8.x_GLF_uniform_int_values[0].el;
      float x_46 = x_6.x_GLF_uniform_float_values[1].el;
      color[x_44] = x_46;
      break;
    }
    default:
    {
      break;
    }
  }
  vec4 x_48 = color;
  x_GLF_color = x_48;
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
