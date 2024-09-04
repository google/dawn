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
  s.arr[1] = (x + 1);
  if ((s.arr[x_9.one] == x)) {
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
      if ((i < (2 + x_9.one))) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          if ((j < (3 + x_9.one))) {
          } else {
            break;
          }
          int x_28 = i;
          int x_29 = j;
          param = s_1;
          param_1 = (x_28 + x_29);
          int x_31 = func_struct_S_i1_2_1_i1_(param, param_1);
          a = (a + x_31);
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((a == 30)) {
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
