SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct S {
  int arr[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
int func_struct_S_i1_2_1_i1_(inout S s, inout int x) {
  int x_16 = x;
  s.arr[1] = (x_16 + 1);
  int x_18 = x_9.one;
  int x_19 = s.arr[x_18];
  int x_20 = x;
  if ((x_19 == x_20)) {
    return -1;
  }
  int x_21 = x;
  return x_21;
}
void main_1() {
  int a = 0;
  int i = 0;
  int j = 0;
  S s_1 = S(int[2](0, 0));
  S param = S(int[2](0, 0));
  int param_1 = 0;
  a = 0;
  i = 0;
  {
    while(true) {
      int x_22 = i;
      int x_23 = x_9.one;
      if ((x_22 < (2 + x_23))) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          int x_25 = j;
          int x_26 = x_9.one;
          if ((x_25 < (3 + x_26))) {
          } else {
            break;
          }
          int x_28 = i;
          int x_29 = j;
          S x_79 = s_1;
          param = x_79;
          param_1 = (x_28 + x_29);
          int x_31 = func_struct_S_i1_2_1_i1_(param, param_1);
          int x_32 = a;
          a = (x_32 + x_31);
          {
            int x_34 = j;
            j = (x_34 + 1);
          }
          continue;
        }
      }
      {
        int x_36 = i;
        i = (x_36 + 1);
      }
      continue;
    }
  }
  int x_38 = a;
  if ((x_38 == 30)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
