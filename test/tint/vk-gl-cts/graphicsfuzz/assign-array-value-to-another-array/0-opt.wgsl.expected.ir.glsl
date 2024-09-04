SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void func_i1_(inout int x) {
  int a = 0;
  int data[9] = int[9](0, 0, 0, 0, 0, 0, 0, 0, 0);
  int temp[2] = int[2](0, 0);
  int i = 0;
  bool x_95 = false;
  bool x_96_phi = false;
  a = 0;
  data[0] = 5;
  {
    while(true) {
      int x_56 = a;
      int x_57 = x;
      if ((x_56 <= x_57)) {
      } else {
        break;
      }
      int x_60 = a;
      if ((x_60 <= 10)) {
        int x_64 = a;
        int x_66 = a;
        int x_69 = data[min(x_66, 0)];
        temp[min(x_64, 1)] = x_69;
        int x_71 = a;
        a = (x_71 + 1);
      }
      {
      }
      continue;
    }
  }
  i = 0;
  {
    while(true) {
      int x_77 = i;
      if ((x_77 < 2)) {
      } else {
        break;
      }
      int x_80 = i;
      int x_82 = temp[0];
      int x_83 = i;
      data[x_80] = (x_82 + x_83);
      {
        int x_86 = i;
        i = (x_86 + 1);
      }
      continue;
    }
  }
  int x_89 = data[0];
  bool x_90 = (x_89 == 5);
  x_96_phi = x_90;
  if (x_90) {
    int x_94 = data[1];
    x_95 = (x_94 == 6);
    x_96_phi = x_95;
  }
  bool x_96 = x_96_phi;
  if (x_96) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
void main_1() {
  int i_1 = 0;
  int param = 0;
  i_1 = 1;
  {
    while(true) {
      int x_43 = i_1;
      if ((x_43 < 6)) {
      } else {
        break;
      }
      int x_46 = i_1;
      param = x_46;
      func_i1_(param);
      {
        int x_48 = i_1;
        i_1 = (x_48 + 1);
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
