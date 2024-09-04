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


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_8;
void main_1() {
  float x_31 = x_6.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4(x_31, x_31, x_31, x_31);
  float x_34 = tint_symbol.y;
  float x_36 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_34 >= x_36)) {
    int x_41 = x_8.x_GLF_uniform_int_values[1].el;
    switch(x_41) {
      case 0:
      case 16:
      {
        int x_45 = x_8.x_GLF_uniform_int_values[0].el;
        float x_46 = float(x_45);
        float x_47 = float(x_41);
        x_GLF_color = vec4(x_46, x_47, x_47, x_46);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  int x_50 = x_8.x_GLF_uniform_int_values[1].el;
  int x_52 = x_8.x_GLF_uniform_int_values[0].el;
  if ((x_50 == x_52)) {
    x_GLF_color = vec4(x_36, x_36, x_36, x_36);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
