SKIP: FAILED

#version 310 es

struct buf0 {
  int zero;
};

struct Array {
  int values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  bool x_52 = false;
  int x_17 = 0;
  int x_18 = 0;
  int x_16[2] = int[2](0, 0);
  Array param = Array(int[2](0, 0));
  int x_20 = 0;
  int x_21 = 0;
  int x_12 = x_8.zero;
  int x_22[2] = x_16;
  int x_23_1[2] = x_22;
  x_23_1[0u] = x_12;
  x_16 = x_23_1;
  param = Array(x_16);
  x_52 = false;
  {
    while(true) {
      bool x_67 = false;
      {
        while(true) {
          if ((param.values[x_12] == 0)) {
            x_52 = true;
            x_17 = 42;
            x_20 = 42;
            x_67 = true;
            break;
          }
          x_20 = 0;
          x_67 = false;
          break;
        }
      }
      x_21 = x_20;
      if (x_67) {
        break;
      }
      x_52 = true;
      x_17 = 42;
      x_21 = 42;
      break;
    }
  }
  x_18 = x_21;
  if ((x_21 == 42)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
int func_struct_Array_i1_2_1_(inout Array a) {
  bool x_74 = false;
  int x_13 = 0;
  bool x_76 = false;
  int x_14 = 0;
  int x_15 = 0;
  x_76 = false;
  {
    while(true) {
      bool x_81 = false;
      bool x_91 = false;
      x_81 = x_76;
      {
        while(true) {
          if ((a.values[x_8.zero] == 0)) {
            x_74 = true;
            x_13 = 42;
            x_14 = 42;
            x_91 = true;
            break;
          }
          x_14 = 0;
          x_91 = x_81;
          break;
        }
      }
      x_15 = x_14;
      if (x_91) {
        break;
      }
      x_74 = true;
      x_13 = 42;
      x_15 = 42;
      break;
    }
  }
  return x_15;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
