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


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int b = 0;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int indexable[2] = int[2](0, 0);
  int x_36 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_36;
  int x_38 = x_6.x_GLF_uniform_int_values[3].el;
  b = x_38;
  int x_40 = x_6.x_GLF_uniform_int_values[2].el;
  float x_41 = float(x_40);
  x_GLF_color = vec4(x_41, x_41, x_41, x_41);
  int x_44 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_44;
  {
    while(true) {
      int x_49 = i;
      int x_51 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_49 < x_51)) {
      } else {
        break;
      }
      int x_54 = i;
      int x_56 = x_6.x_GLF_uniform_int_values[3].el;
      if ((x_54 > x_56)) {
        int x_60 = a;
        a = (x_60 + 1);
        if (false) {
          int x_65 = x_6.x_GLF_uniform_int_values[2].el;
          i_1 = x_65;
          {
            while(true) {
              int x_70 = i_1;
              int x_72 = x_6.x_GLF_uniform_int_values[0].el;
              if ((x_70 < x_72)) {
              } else {
                break;
              }
              return;
            }
          }
        }
      }
      {
        int x_75 = i;
        i = (x_75 + 1);
      }
      continue;
    }
  }
  int x_78 = x_6.x_GLF_uniform_int_values[2].el;
  i_2 = x_78;
  {
    while(true) {
      int x_83 = i_2;
      int x_85 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_83 < x_85)) {
      } else {
        break;
      }
      int x_89 = x_6.x_GLF_uniform_int_values[3].el;
      int x_91 = x_6.x_GLF_uniform_int_values[4].el;
      int x_93 = b;
      indexable = int[2](x_89, x_91);
      int x_95 = indexable[x_93];
      int x_96 = a;
      a = (x_96 + x_95);
      {
        int x_98 = i_2;
        i_2 = (x_98 + 1);
      }
      continue;
    }
  }
  int x_100 = a;
  int x_102 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_100 == x_102)) {
    int x_107 = x_6.x_GLF_uniform_int_values[3].el;
    int x_110 = x_6.x_GLF_uniform_int_values[2].el;
    int x_113 = x_6.x_GLF_uniform_int_values[2].el;
    int x_116 = x_6.x_GLF_uniform_int_values[3].el;
    float v = float(x_107);
    float v_1 = float(x_110);
    float v_2 = float(x_113);
    x_GLF_color = vec4(v, v_1, v_2, float(x_116));
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
