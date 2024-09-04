SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  S A[2] = S[2](S(0, 0, 0), S(0, 0, 0));
  int x_29 = x_7.x_GLF_uniform_int_values[1].el;
  int x_31 = x_7.x_GLF_uniform_int_values[1].el;
  int x_33 = x_7.x_GLF_uniform_int_values[1].el;
  int x_35 = x_7.x_GLF_uniform_int_values[1].el;
  A[x_29] = S(x_31, x_33, x_35);
  int x_39 = x_7.x_GLF_uniform_int_values[0].el;
  int x_41 = x_7.x_GLF_uniform_int_values[1].el;
  int x_43 = x_7.x_GLF_uniform_int_values[1].el;
  int x_45 = x_7.x_GLF_uniform_int_values[1].el;
  A[x_39] = S(x_41, x_43, x_45);
  int x_49 = x_7.x_GLF_uniform_int_values[1].el;
  int x_51 = A[x_49].b;
  int x_53 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_51 == x_53)) {
    int x_58 = x_7.x_GLF_uniform_int_values[1].el;
    int x_61 = x_7.x_GLF_uniform_int_values[0].el;
    A[min(max(x_58, 1), 2)].b = x_61;
  }
  int x_64 = x_7.x_GLF_uniform_int_values[0].el;
  int x_66 = A[x_64].b;
  int x_68 = x_7.x_GLF_uniform_int_values[0].el;
  if ((x_66 == x_68)) {
    int x_74 = x_7.x_GLF_uniform_int_values[0].el;
    int x_77 = x_7.x_GLF_uniform_int_values[1].el;
    int x_80 = x_7.x_GLF_uniform_int_values[1].el;
    int x_83 = x_7.x_GLF_uniform_int_values[0].el;
    float v = float(x_74);
    float v_1 = float(x_77);
    float v_2 = float(x_80);
    x_GLF_color = vec4(v, v_1, v_2, float(x_83));
  } else {
    int x_87 = x_7.x_GLF_uniform_int_values[0].el;
    float x_88 = float(x_87);
    x_GLF_color = vec4(x_88, x_88, x_88, x_88);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
