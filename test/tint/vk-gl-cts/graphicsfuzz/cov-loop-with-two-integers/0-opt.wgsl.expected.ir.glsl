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
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float arr[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  int j = 0;
  float x_38 = x_6.x_GLF_uniform_float_values[0].el;
  float x_40 = x_6.x_GLF_uniform_float_values[0].el;
  float x_42 = x_6.x_GLF_uniform_float_values[0].el;
  float x_44 = x_6.x_GLF_uniform_float_values[0].el;
  float x_46 = x_6.x_GLF_uniform_float_values[0].el;
  arr = float[5](x_38, x_40, x_42, x_44, x_46);
  int x_49 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_49;
  j = 0;
  {
    while(true) {
      int x_54 = i;
      int x_56 = x_9.x_GLF_uniform_int_values[0].el;
      if ((x_54 < x_56)) {
      } else {
        break;
      }
      int x_59 = j;
      if ((x_59 < -1)) {
        break;
      }
      int x_63 = j;
      float x_65 = arr[x_63];
      arr[x_63] = (x_65 + 1.0f);
      {
        int x_68 = i;
        i = (x_68 + 1);
        int x_70 = j;
        j = (x_70 + 1);
      }
      continue;
    }
  }
  float x_73 = x_6.x_GLF_uniform_float_values[0].el;
  float x_75 = x_6.x_GLF_uniform_float_values[1].el;
  float x_77 = x_6.x_GLF_uniform_float_values[1].el;
  float x_79 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4(x_73, x_75, x_77, x_79);
  int x_82 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_82;
  {
    while(true) {
      int x_87 = i;
      int x_89 = x_9.x_GLF_uniform_int_values[0].el;
      if ((x_87 < x_89)) {
      } else {
        break;
      }
      int x_92 = i;
      float x_94 = arr[x_92];
      if (!((x_94 == 2.0f))) {
        float x_99 = x_6.x_GLF_uniform_float_values[1].el;
        x_GLF_color = vec4(x_99, x_99, x_99, x_99);
      }
      {
        int x_101 = i;
        i = (x_101 + 1);
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
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
