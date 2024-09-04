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
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_11;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  int i = 0;
  float x_35 = x_6.x_GLF_uniform_float_values[1].el;
  a = x_35;
  float x_37 = x_6.x_GLF_uniform_float_values[1].el;
  b = x_37;
  float x_39 = x_6.x_GLF_uniform_float_values[1].el;
  c = x_39;
  int x_41 = x_11.x_GLF_uniform_int_values[1].el;
  i = x_41;
  {
    while(true) {
      int x_46 = i;
      int x_48 = x_11.x_GLF_uniform_int_values[0].el;
      if ((x_46 < x_48)) {
      } else {
        break;
      }
      int x_51 = i;
      int x_53 = x_11.x_GLF_uniform_int_values[2].el;
      if ((x_51 == x_53)) {
        float x_57 = a;
        float x_60 = x_6.x_GLF_uniform_float_values[1].el;
        b = (dFdx(x_57) + x_60);
      }
      float x_62 = a;
      c = dFdx(x_62);
      float x_64 = c;
      float x_65 = b;
      a = (x_64 / x_65);
      {
        int x_67 = i;
        i = (x_67 + 1);
      }
      continue;
    }
  }
  float x_69 = a;
  float x_71 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_69 == x_71)) {
    int x_77 = x_11.x_GLF_uniform_int_values[2].el;
    int x_80 = x_11.x_GLF_uniform_int_values[1].el;
    int x_83 = x_11.x_GLF_uniform_int_values[1].el;
    int x_86 = x_11.x_GLF_uniform_int_values[2].el;
    float v = float(x_77);
    float v_1 = float(x_80);
    float v_2 = float(x_83);
    x_GLF_color = vec4(v, v_1, v_2, float(x_86));
  } else {
    int x_90 = x_11.x_GLF_uniform_int_values[1].el;
    float x_91 = float(x_90);
    x_GLF_color = vec4(x_91, x_91, x_91, x_91);
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
