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
  int x_20 = 0;
  param = Array(int[2](0, 0));
  x_50 = false;
  {
    while(true) {
      bool x_63 = false;
      {
        while(true) {
          if ((param.values[x_8.zero] == 1)) {
            x_50 = true;
            x_15 = 1;
            x_19 = 1;
            x_63 = true;
            break;
          }
          x_19 = 0;
          x_63 = false;
          break;
        }
      }
      x_20 = x_19;
      if (x_63) {
        break;
      }
      x_50 = true;
      x_15 = 1;
      x_20 = 1;
      break;
    }
  }
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
  bool x_72 = false;
  int x_13 = 0;
  int x_14 = 0;
  x_72 = false;
  {
    while(true) {
      bool x_77 = false;
      bool x_87 = false;
      x_77 = x_72;
      {
        while(true) {
          if ((a.values[x_8.zero] == 1)) {
            x_70 = true;
            x_12 = 1;
            x_13 = 1;
            x_87 = true;
            break;
          }
          x_13 = 0;
          x_87 = x_77;
          break;
        }
      }
      x_14 = x_13;
      if (x_87) {
        break;
      }
      x_70 = true;
      x_12 = 1;
      x_14 = 1;
      break;
    }
  }
  return x_14;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
