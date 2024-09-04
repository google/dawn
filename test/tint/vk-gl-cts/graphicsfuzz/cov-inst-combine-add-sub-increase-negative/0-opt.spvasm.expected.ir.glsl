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


uniform buf1 x_7;
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_11;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int i = 0;
  int arr[2] = int[2](0, 0);
  int a = 0;
  i = x_7.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_7.x_GLF_uniform_int_values[2].el)) {
      } else {
        break;
      }
      int x_50 = i;
      arr[x_50] = x_7.x_GLF_uniform_int_values[0].el;
      {
        i = (i + 1);
      }
      continue;
    }
  }
  a = -1;
  if (!((tint_symbol.y < x_11.x_GLF_uniform_float_values[0].el))) {
    int x_65 = (a + 1);
    a = x_65;
    arr[x_65] = x_7.x_GLF_uniform_int_values[1].el;
  }
  int x_70 = (a + 1);
  a = x_70;
  arr[x_70] = x_7.x_GLF_uniform_int_values[2].el;
  if ((arr[x_7.x_GLF_uniform_int_values[0].el] == x_7.x_GLF_uniform_int_values[1].el)) {
    float v = float(a);
    float v_1 = float(x_7.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_7.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(a));
  } else {
    x_GLF_color = vec4(float(x_7.x_GLF_uniform_int_values[0].el));
  }
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
