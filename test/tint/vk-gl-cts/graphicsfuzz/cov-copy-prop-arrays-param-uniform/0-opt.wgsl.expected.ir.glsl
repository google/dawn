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
  int x_21_phi = 0;
  int x_12 = x_8.zero;
  int x_22[2] = x_16;
  int x_23_1[2] = x_22;
  x_23_1[0u] = x_12;
  int x_23[2] = x_23_1;
  x_16 = x_23;
  int x_54[2] = x_16;
  param = Array(x_54);
  x_52 = false;
  {
    while(true) {
      int x_20_phi = 0;
      bool x_67_phi = false;
      {
        while(true) {
          int x_19 = param.values[x_12];
          if ((x_19 == 0)) {
            x_52 = true;
            x_17 = 42;
            x_20_phi = 42;
            x_67_phi = true;
            break;
          }
          x_20_phi = 0;
          x_67_phi = false;
          break;
        }
      }
      x_20 = x_20_phi;
      bool x_67 = x_67_phi;
      x_21_phi = x_20;
      if (x_67) {
        break;
      }
      x_52 = true;
      x_17 = 42;
      x_21_phi = 42;
      break;
    }
  }
  int x_21 = x_21_phi;
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
  int x_14 = 0;
  bool x_76_phi = false;
  int x_15_phi = 0;
  x_76_phi = false;
  {
    while(true) {
      bool x_81 = false;
      bool x_81_phi = false;
      int x_14_phi = 0;
      bool x_91_phi = false;
      bool x_76 = x_76_phi;
      x_81_phi = x_76;
      {
        while(true) {
          x_81 = x_81_phi;
          int x_10 = x_8.zero;
          int x_11 = a.values[x_10];
          if ((x_11 == 0)) {
            x_74 = true;
            x_13 = 42;
            x_14_phi = 42;
            x_91_phi = true;
            break;
          }
          x_14_phi = 0;
          x_91_phi = x_81;
          break;
        }
      }
      x_14 = x_14_phi;
      bool x_91 = x_91_phi;
      x_15_phi = x_14;
      if (x_91) {
        break;
      }
      x_74 = true;
      x_13 = 42;
      x_15_phi = 42;
      break;
    }
  }
  int x_15 = x_15_phi;
  return x_15;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
