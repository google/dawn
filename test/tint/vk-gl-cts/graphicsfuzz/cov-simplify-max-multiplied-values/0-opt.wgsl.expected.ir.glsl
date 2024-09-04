SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int i = 0;
  int A[4] = int[4](0, 0, 0, 0);
  bool x_77 = false;
  bool x_87 = false;
  bool x_97 = false;
  bool x_78_phi = false;
  bool x_88_phi = false;
  bool x_98_phi = false;
  int x_33 = x_6.x_GLF_uniform_int_values[0].el;
  i = x_33;
  {
    while(true) {
      int x_38 = i;
      int x_40 = x_6.x_GLF_uniform_int_values[4].el;
      if ((x_38 < x_40)) {
      } else {
        break;
      }
      int x_43 = i;
      int x_45 = x_6.x_GLF_uniform_int_values[0].el;
      A[x_43] = x_45;
      int x_47 = i;
      int x_50 = x_6.x_GLF_uniform_int_values[3].el;
      int x_54 = x_6.x_GLF_uniform_int_values[1].el;
      if ((max((2 * x_47), (2 * x_50)) == x_54)) {
        int x_58 = i;
        A[x_58] = 1;
      }
      {
        int x_60 = i;
        i = (x_60 + 1);
      }
      continue;
    }
  }
  int x_63 = x_6.x_GLF_uniform_int_values[0].el;
  int x_65 = A[x_63];
  int x_67 = x_6.x_GLF_uniform_int_values[3].el;
  bool x_68 = (x_65 == x_67);
  x_78_phi = x_68;
  if (x_68) {
    int x_72 = x_6.x_GLF_uniform_int_values[3].el;
    int x_74 = A[x_72];
    int x_76 = x_6.x_GLF_uniform_int_values[3].el;
    x_77 = (x_74 == x_76);
    x_78_phi = x_77;
  }
  bool x_78 = x_78_phi;
  x_88_phi = x_78;
  if (x_78) {
    int x_82 = x_6.x_GLF_uniform_int_values[1].el;
    int x_84 = A[x_82];
    int x_86 = x_6.x_GLF_uniform_int_values[0].el;
    x_87 = (x_84 == x_86);
    x_88_phi = x_87;
  }
  bool x_88 = x_88_phi;
  x_98_phi = x_88;
  if (x_88) {
    int x_92 = x_6.x_GLF_uniform_int_values[2].el;
    int x_94 = A[x_92];
    int x_96 = x_6.x_GLF_uniform_int_values[0].el;
    x_97 = (x_94 == x_96);
    x_98_phi = x_97;
  }
  bool x_98 = x_98_phi;
  if (x_98) {
    int x_103 = x_6.x_GLF_uniform_int_values[3].el;
    int x_106 = x_6.x_GLF_uniform_int_values[0].el;
    int x_109 = x_6.x_GLF_uniform_int_values[0].el;
    int x_112 = x_6.x_GLF_uniform_int_values[3].el;
    float v = float(x_103);
    float v_1 = float(x_106);
    float v_2 = float(x_109);
    x_GLF_color = vec4(v, v_1, v_2, float(x_112));
  } else {
    x_GLF_color = vec4(1.0f);
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
