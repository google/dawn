SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
int func_i1_(inout int x) {
  int a = 0;
  int i = 0;
  int indexable[4] = int[4](0, 0, 0, 0);
  int x_72 = x_8.x_GLF_uniform_int_values[0].el;
  a = x_72;
  int x_74 = x_8.x_GLF_uniform_int_values[0].el;
  i = x_74;
  {
    while(true) {
      int x_79 = i;
      int x_81 = x_8.x_GLF_uniform_int_values[1].el;
      if ((x_79 < x_81)) {
      } else {
        break;
      }
      int x_85 = x_8.x_GLF_uniform_int_values[3].el;
      int x_87 = x_8.x_GLF_uniform_int_values[3].el;
      int x_89 = x_8.x_GLF_uniform_int_values[3].el;
      int x_91 = x_8.x_GLF_uniform_int_values[3].el;
      int x_93 = a;
      indexable = int[4](x_85, x_87, x_89, x_91);
      int x_95 = indexable[x_93];
      int x_96 = x;
      if ((x_95 > x_96)) {
        if (true) {
          int x_105 = x_8.x_GLF_uniform_int_values[3].el;
          return x_105;
        } else {
          int x_107 = x_8.x_GLF_uniform_int_values[3].el;
          a = x_107;
        }
      } else {
        if (true) {
          int x_111 = x_8.x_GLF_uniform_int_values[4].el;
          return x_111;
        }
      }
      {
        int x_112 = i;
        i = (x_112 + 1);
      }
      continue;
    }
  }
  int x_115 = x_8.x_GLF_uniform_int_values[0].el;
  return x_115;
}
void main_1() {
  int a_1 = 0;
  int param = 0;
  int param_1 = 0;
  int x_39 = x_8.x_GLF_uniform_int_values[0].el;
  param = x_39;
  int x_40 = func_i1_(param);
  a_1 = x_40;
  int x_42 = x_8.x_GLF_uniform_int_values[3].el;
  param_1 = x_42;
  int x_43 = func_i1_(param_1);
  int x_44 = a_1;
  a_1 = (x_44 + x_43);
  int x_46 = a_1;
  int x_48 = x_8.x_GLF_uniform_int_values[2].el;
  if ((x_46 == x_48)) {
    int x_54 = x_8.x_GLF_uniform_int_values[3].el;
    int x_57 = x_8.x_GLF_uniform_int_values[0].el;
    int x_60 = x_8.x_GLF_uniform_int_values[0].el;
    int x_63 = x_8.x_GLF_uniform_int_values[3].el;
    float v = float(x_54);
    float v_1 = float(x_57);
    float v_2 = float(x_60);
    x_GLF_color = vec4(v, v_1, v_2, float(x_63));
  } else {
    int x_67 = x_8.x_GLF_uniform_int_values[0].el;
    float x_68 = float(x_67);
    x_GLF_color = vec4(x_68, x_68, x_68, x_68);
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
