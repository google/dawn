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
uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int i = 0;
  float b = 0.0f;
  int x_34 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_34;
  int x_35 = a;
  a = (x_35 + 1);
  int x_38 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_38;
  {
    while(true) {
      int x_43 = i;
      int x_45 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_43 < x_45)) {
      } else {
        break;
      }
      int x_48 = i;
      int x_50 = a;
      b = ldexp(float(x_48), -(x_50));
      {
        int x_53 = i;
        i = (x_53 + 1);
      }
      continue;
    }
  }
  float x_55 = b;
  float x_57 = x_10.x_GLF_uniform_float_values[0].el;
  if ((x_55 == x_57)) {
    int x_63 = x_6.x_GLF_uniform_int_values[2].el;
    int x_66 = x_6.x_GLF_uniform_int_values[1].el;
    int x_69 = x_6.x_GLF_uniform_int_values[1].el;
    int x_72 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_63);
    float v_1 = float(x_66);
    float v_2 = float(x_69);
    x_GLF_color = vec4(v, v_1, v_2, float(x_72));
  } else {
    float x_75 = b;
    x_GLF_color = vec4(x_75, x_75, x_75, x_75);
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
