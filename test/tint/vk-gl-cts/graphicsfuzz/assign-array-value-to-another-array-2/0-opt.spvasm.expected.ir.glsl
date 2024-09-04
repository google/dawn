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
  bool x_116 = false;
  b = 0;
  data[0] = 5;
  data[2] = 0;
  data[4] = 0;
  data[6] = 0;
  data[8] = 0;
  if ((tint_symbol.x >= 0.0f)) {
    {
      while(true) {
        if ((b <= a)) {
        } else {
          break;
        }
        if ((b <= 5)) {
          int x_87 = b;
          temp[x_87] = data[b];
          b = (b + 2);
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
      if ((i < 3)) {
      } else {
        break;
      }
      int x_101 = i;
      data[x_101] = (temp[0] + 1);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  bool x_110 = (temp[0] == 5);
  x_116 = x_110;
  if (x_110) {
    x_115 = (data[0] == 6);
    x_116 = x_115;
  }
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
      if ((i_1 < 6)) {
      } else {
        break;
      }
      param = i_1;
      float x_55 = func_i1_(param);
      param_1 = i_1;
      float x_57 = func_i1_(param_1);
      if ((x_57 == 1.0f)) {
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      } else {
        x_GLF_color = vec4(0.0f);
      }
      {
        i_1 = (i_1 + 1);
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
