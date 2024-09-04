SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_5;
uniform buf0 x_8;
void main_1() {
  int i = 0;
  int x_29 = x_5.x_GLF_uniform_int_values[0].el;
  float x_30 = float(x_29);
  x_GLF_color = vec4(x_30, x_30, x_30, x_30);
  int x_33 = x_5.x_GLF_uniform_int_values[0].el;
  i = x_33;
  {
    while(true) {
      int x_38 = i;
      int x_40 = x_5.x_GLF_uniform_int_values[1].el;
      if ((x_38 < x_40)) {
      } else {
        break;
      }
      float x_44 = x_8.x_GLF_uniform_float_values[1].el;
      int x_45 = i;
      if (!((x_44 <= float(x_45)))) {
        float x_52 = x_8.x_GLF_uniform_float_values[0].el;
        int x_53 = i;
        int x_55 = i;
        float x_58 = x_8.x_GLF_uniform_float_values[0].el;
        vec4 x_60 = x_GLF_color;
        float v = float(x_53);
        x_GLF_color = (x_60 + vec4(x_52, v, float(x_55), x_58));
      }
      {
        int x_62 = i;
        i = (x_62 + 1);
      }
      continue;
    }
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
