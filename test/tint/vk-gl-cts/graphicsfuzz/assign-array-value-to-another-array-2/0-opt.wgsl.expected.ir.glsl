SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int data[9] = int[9](0, 0, 0, 0, 0, 0, 0, 0, 0);
vec4 tint_symbol = vec4(0.0f);
int temp[7] = int[7](0, 0, 0, 0, 0, 0, 0);
vec4 x_GLF_color = vec4(0.0f);
float func_i1_(inout int a) {
  int b = 0;
  int i = 0;
  bool x_115 = false;
  bool x_116_phi = false;
  b = 0;
  data[0] = 5;
  data[2] = 0;
  data[4] = 0;
  data[6] = 0;
  data[8] = 0;
  float x_71 = tint_symbol.x;
  if ((x_71 >= 0.0f)) {
    {
      while(true) {
        int x_79 = b;
        int x_80 = a;
        if ((x_79 <= x_80)) {
        } else {
          break;
        }
        int x_83 = b;
        if ((x_83 <= 5)) {
          int x_87 = b;
          int x_88 = b;
          int x_90 = data[x_88];
          temp[x_87] = x_90;
          int x_92 = b;
          b = (x_92 + 2);
        }
        {
        }
        continue;
      }
    }
  }
  i = 0;
  {
    while(true) {
      int x_98 = i;
      if ((x_98 < 3)) {
      } else {
        break;
      }
      int x_101 = i;
      int x_103 = temp[0];
      data[x_101] = (x_103 + 1);
      {
        int x_106 = i;
        i = (x_106 + 1);
      }
      continue;
    }
  }
  int x_109 = temp[0];
  bool x_110 = (x_109 == 5);
  x_116_phi = x_110;
  if (x_110) {
    int x_114 = data[0];
    x_115 = (x_114 == 6);
    x_116_phi = x_115;
  }
  bool x_116 = x_116_phi;
  if (x_116) {
    return 1.0f;
  } else {
    return 0.0f;
  }
  /* unreachable */
}
void main_1() {
  int i_1 = 0;
  int param = 0;
  int param_1 = 0;
  i_1 = 0;
  {
    while(true) {
      int x_51 = i_1;
      if ((x_51 < 6)) {
      } else {
        break;
      }
      int x_54 = i_1;
      param = x_54;
      float x_55 = func_i1_(param);
      int x_56 = i_1;
      param_1 = x_56;
      float x_57 = func_i1_(param_1);
      if ((x_57 == 1.0f)) {
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      } else {
        x_GLF_color = vec4(0.0f);
      }
      {
        int x_62 = i_1;
        i_1 = (x_62 + 1);
      }
      continue;
    }
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
