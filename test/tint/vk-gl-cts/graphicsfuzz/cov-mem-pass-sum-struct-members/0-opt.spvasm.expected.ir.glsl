SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct S {
  int a;
  int b;
  int c;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int x_43 = 0;
  bool x_44 = false;
  S arr[2] = S[2](S(0, 0, 0), S(0, 0, 0));
  S param = S(0, 0, 0);
  int param_1 = 0;
  {
    while(true) {
      int x_50 = 0;
      x_50 = x_10.one;
      arr[x_50].a = 2;
      if ((arr[1].a < 1)) {
        x_GLF_color = vec4(0.0f);
        x_44 = true;
        break;
      } else {
        param = arr[1];
        param_1 = (2 + x_50);
        int x_61 = param_1;
        S x_63 = param;
        S x_64_1 = x_63;
        x_64_1.a = x_61;
        param = x_64_1;
        if ((param.a == 2)) {
          S x_70 = param;
          S x_71_1 = x_70;
          x_71_1.a = 9;
          param = x_71_1;
        }
        int x_72 = param_1;
        S x_75 = param;
        S x_76_1 = x_75;
        x_76_1.b = (x_72 + 1);
        param = x_76_1;
        int x_77 = param_1;
        S x_80 = param;
        S x_81_1 = x_80;
        x_81_1.c = (x_77 + 2);
        param = x_81_1;
        if ((param.b == 2)) {
          S x_87 = param;
          S x_88_1 = x_87;
          x_88_1.b = 7;
          param = x_88_1;
        }
        x_43 = ((param.a + param.b) + param.c);
        if ((x_43 == 12)) {
          x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        } else {
          x_GLF_color = vec4(0.0f);
        }
      }
      x_44 = true;
      break;
    }
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
int func_struct_S_i1_i1_i11_i1_(inout S s, inout int x) {
  s.a = x;
  if ((s.a == 2)) {
    s.a = 9;
  }
  s.b = (x + 1);
  s.c = (x + 2);
  if ((s.b == 2)) {
    s.b = 7;
  }
  int x_119 = s.a;
  int x_120 = s.b;
  int x_122 = s.c;
  return ((x_119 + x_120) + x_122);
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
