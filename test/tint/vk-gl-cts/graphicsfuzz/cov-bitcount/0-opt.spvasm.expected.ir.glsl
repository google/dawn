SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_8;
uniform buf1 x_11;
vec4 x_GLF_color = vec4(0.0f);
int f1_() {
  int a = 0;
  int i = 0;
  a = 256;
  if ((tint_symbol.y > x_8.x_GLF_uniform_float_values[0].el)) {
    a = (a + 1);
  }
  i = bitCount(a);
  if ((i < x_11.x_GLF_uniform_int_values[0].el)) {
    int x_82 = x_11.x_GLF_uniform_int_values[0].el;
    return x_82;
  }
  int x_83 = i;
  return x_83;
}
void main_1() {
  int a_1 = 0;
  int x_38 = f1_();
  a_1 = x_38;
  if ((a_1 == x_11.x_GLF_uniform_int_values[2].el)) {
    float v = float(x_11.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_11.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_11.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_11.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_11.x_GLF_uniform_int_values[1].el));
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
