SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct S {
  int x;
  int y;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
void func_struct_S_i1_i11_(inout S arg) {
  arg.y = 1;
}
void main_1() {
  float a = 0.0f;
  S b[2] = S[2](S(0, 0), S(0, 0));
  S param = S(0, 0);
  a = 5.0f;
  {
    while(true) {
      int x_43 = x_10.one;
      b[x_43].x = 1;
      int x_46 = b[1].x;
      if ((x_46 == 1)) {
        int x_51 = x_10.one;
        if ((x_51 == 1)) {
          break;
        }
        S x_56 = b[1];
        param = x_56;
        func_struct_S_i1_i11_(param);
        S x_58 = param;
        b[1] = x_58;
        int x_61 = b[1].y;
        a = float(x_61);
      }
      a = 0.0f;
      {
        if (true) { break; }
      }
      continue;
    }
  }
  float x_63 = a;
  if ((x_63 == 5.0f)) {
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
ERROR: 0:13: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
