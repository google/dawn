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
  bool x_50 = false;
  int x_15 = 0;
  int x_16 = 0;
  Array param = Array(int[2](0, 0));
  int x_19 = 0;
  int x_20_phi = 0;
  param = Array(int[2](0, 0));
  x_50 = false;
  {
    while(true) {
      int x_19_phi = 0;
      bool x_63_phi = false;
      {
        while(true) {
          int x_17 = x_8.zero;
          int x_18 = param.values[x_17];
          if ((x_18 == 1)) {
            x_50 = true;
            x_15 = 1;
            x_19_phi = 1;
            x_63_phi = true;
            break;
          }
          x_19_phi = 0;
          x_63_phi = false;
          break;
        }
      }
      x_19 = x_19_phi;
      bool x_63 = x_63_phi;
      x_20_phi = x_19;
      if (x_63) {
        break;
      }
      x_50 = true;
      x_15 = 1;
      x_20_phi = 1;
      break;
    }
  }
  int x_20 = x_20_phi;
  x_16 = x_20;
  if ((x_20 == 1)) {
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
  bool x_70 = false;
  int x_12 = 0;
  int x_13 = 0;
  bool x_72_phi = false;
  int x_14_phi = 0;
  x_72_phi = false;
  {
    while(true) {
      bool x_77 = false;
      bool x_77_phi = false;
      int x_13_phi = 0;
      bool x_87_phi = false;
      bool x_72 = x_72_phi;
      x_77_phi = x_72;
      {
        while(true) {
          x_77 = x_77_phi;
          int x_10 = x_8.zero;
          int x_11 = a.values[x_10];
          if ((x_11 == 1)) {
            x_70 = true;
            x_12 = 1;
            x_13_phi = 1;
            x_87_phi = true;
            break;
          }
          x_13_phi = 0;
          x_87_phi = x_77;
          break;
        }
      }
      x_13 = x_13_phi;
      bool x_87 = x_87_phi;
      x_14_phi = x_13;
      if (x_87) {
        break;
      }
      x_70 = true;
      x_12 = 1;
      x_14_phi = 1;
      break;
    }
  }
  int x_14 = x_14_phi;
  return x_14;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
