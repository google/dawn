SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_8;
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_12;
vec4 x_GLF_color = vec4(0.0f);
int f1_() {
  int i = 0;
  int A[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int a = 0;
  i = x_8.x_GLF_uniform_int_values[2].el;
  {
    while(true) {
      if ((i < x_8.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      int x_66 = i;
      A[x_66] = x_8.x_GLF_uniform_int_values[2].el;
      {
        i = (i + 1);
      }
      continue;
    }
  }
  a = -1;
  if ((tint_symbol.y >= x_12.x_GLF_uniform_float_values[0].el)) {
    int x_80 = (a + 1);
    a = x_80;
    A[x_80] = x_8.x_GLF_uniform_int_values[1].el;
  }
  if ((A[x_8.x_GLF_uniform_int_values[2].el] == x_8.x_GLF_uniform_int_values[1].el)) {
    int x_95 = (a + 1);
    a = x_95;
    int x_97 = A[x_95];
    return x_97;
  } else {
    int x_99 = x_8.x_GLF_uniform_int_values[1].el;
    return x_99;
  }
  /* unreachable */
}
void main_1() {
  int i_1 = 0;
  int x_42 = f1_();
  i_1 = x_42;
  float v = float(x_8.x_GLF_uniform_int_values[1].el);
  float v_1 = float(i_1);
  float v_2 = float(i_1);
  x_GLF_color = vec4(v, v_1, v_2, float(x_8.x_GLF_uniform_int_values[1].el));
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
