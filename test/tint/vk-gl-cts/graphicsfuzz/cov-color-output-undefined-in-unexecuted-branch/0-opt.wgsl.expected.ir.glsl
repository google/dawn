SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf2 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct buf3 {
  int three;
};

struct strided_arr_2 {
  uint el;
};

struct buf0 {
  strided_arr_2 x_GLF_uniform_uint_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
uniform buf2 x_12;
uniform buf3 x_14;
uniform buf0 x_16;
void func0_() {
  vec4 tmp = vec4(0.0f);
  float x_112 = tint_symbol.x;
  float x_114 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_112 > x_114)) {
    vec4 x_118 = x_GLF_color;
    tmp = x_118;
  }
  vec4 x_119 = tmp;
  x_GLF_color = x_119;
}
int func1_() {
  int a = 0;
  int x_122 = x_12.x_GLF_uniform_int_values[1].el;
  a = x_122;
  {
    while(true) {
      int x_127 = a;
      int x_129 = x_12.x_GLF_uniform_int_values[3].el;
      if ((x_127 < x_129)) {
      } else {
        break;
      }
      int x_133 = x_14.three;
      int x_135 = x_12.x_GLF_uniform_int_values[1].el;
      if ((x_133 > x_135)) {
        func0_();
        int x_142 = x_12.x_GLF_uniform_int_values[3].el;
        a = x_142;
      } else {
        func0_();
      }
      {
      }
      continue;
    }
  }
  int x_144 = a;
  return x_144;
}
void main_1() {
  int a_1 = 0;
  int i = 0;
  int j = 0;
  float x_56 = tint_symbol.x;
  float x_58 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_56 > x_58)) {
    float x_64 = x_8.x_GLF_uniform_float_values[0].el;
    float x_66 = x_8.x_GLF_uniform_float_values[1].el;
    float x_68 = x_8.x_GLF_uniform_float_values[0].el;
    float x_70 = x_8.x_GLF_uniform_float_values[2].el;
    x_GLF_color = vec4(x_64, x_66, x_68, x_70);
  } else {
    uint x_73 = x_16.x_GLF_uniform_uint_values[0].el;
    x_GLF_color = unpackSnorm4x8(x_73);
  }
  int x_76 = x_12.x_GLF_uniform_int_values[2].el;
  a_1 = x_76;
  i = 0;
  {
    while(true) {
      int x_81 = i;
      if ((x_81 < 5)) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          int x_88 = j;
          if ((x_88 < 2)) {
          } else {
            break;
          }
          int x_91 = func1_();
          int x_92 = a_1;
          a_1 = (x_92 + x_91);
          {
            int x_94 = j;
            j = (x_94 + 1);
          }
          continue;
        }
      }
      {
        int x_96 = i;
        i = (x_96 + 1);
      }
      continue;
    }
  }
  int x_98 = a_1;
  int x_100 = x_12.x_GLF_uniform_int_values[0].el;
  if ((x_98 == x_100)) {
    float x_105 = x_8.x_GLF_uniform_float_values[0].el;
    float x_107 = x_GLF_color.z;
    x_GLF_color[2u] = (x_107 - x_105);
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
