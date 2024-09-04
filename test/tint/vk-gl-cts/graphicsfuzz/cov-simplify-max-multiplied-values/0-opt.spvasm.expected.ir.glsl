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
  bool x_78 = false;
  bool x_87 = false;
  bool x_88 = false;
  bool x_97 = false;
  bool x_98 = false;
  i = x_6.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_6.x_GLF_uniform_int_values[4].el)) {
      } else {
        break;
      }
      int x_43 = i;
      A[x_43] = x_6.x_GLF_uniform_int_values[0].el;
      int v = max((2 * i), (2 * x_6.x_GLF_uniform_int_values[3].el));
      if ((v == x_6.x_GLF_uniform_int_values[1].el)) {
        int x_58 = i;
        A[x_58] = 1;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  bool x_68 = (A[x_6.x_GLF_uniform_int_values[0].el] == x_6.x_GLF_uniform_int_values[3].el);
  x_78 = x_68;
  if (x_68) {
    x_77 = (A[x_6.x_GLF_uniform_int_values[3].el] == x_6.x_GLF_uniform_int_values[3].el);
    x_78 = x_77;
  }
  x_88 = x_78;
  if (x_78) {
    x_87 = (A[x_6.x_GLF_uniform_int_values[1].el] == x_6.x_GLF_uniform_int_values[0].el);
    x_88 = x_87;
  }
  x_98 = x_88;
  if (x_88) {
    x_97 = (A[x_6.x_GLF_uniform_int_values[2].el] == x_6.x_GLF_uniform_int_values[0].el);
    x_98 = x_97;
  }
  if (x_98) {
    float v_1 = float(x_6.x_GLF_uniform_int_values[3].el);
    float v_2 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_3 = float(x_6.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_6.x_GLF_uniform_int_values[3].el));
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
