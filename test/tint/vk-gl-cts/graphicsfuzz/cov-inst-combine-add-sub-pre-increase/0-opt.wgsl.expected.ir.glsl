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
  int x_56 = x_8.x_GLF_uniform_int_values[2].el;
  i = x_56;
  {
    while(true) {
      int x_61 = i;
      int x_63 = x_8.x_GLF_uniform_int_values[0].el;
      if ((x_61 < x_63)) {
      } else {
        break;
      }
      int x_66 = i;
      int x_68 = x_8.x_GLF_uniform_int_values[2].el;
      A[x_66] = x_68;
      {
        int x_70 = i;
        i = (x_70 + 1);
      }
      continue;
    }
  }
  a = -1;
  float x_73 = tint_symbol.y;
  float x_75 = x_12.x_GLF_uniform_float_values[0].el;
  if ((x_73 >= x_75)) {
    int x_79 = a;
    int x_80 = (x_79 + 1);
    a = x_80;
    int x_82 = x_8.x_GLF_uniform_int_values[1].el;
    A[x_80] = x_82;
  }
  int x_85 = x_8.x_GLF_uniform_int_values[2].el;
  int x_87 = A[x_85];
  int x_89 = x_8.x_GLF_uniform_int_values[1].el;
  if ((x_87 == x_89)) {
    int x_94 = a;
    int x_95 = (x_94 + 1);
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
  int x_44 = x_8.x_GLF_uniform_int_values[1].el;
  int x_46 = i_1;
  int x_48 = i_1;
  int x_51 = x_8.x_GLF_uniform_int_values[1].el;
  float v = float(x_44);
  float v_1 = float(x_46);
  float v_2 = float(x_48);
  x_GLF_color = vec4(v, v_1, v_2, float(x_51));
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
