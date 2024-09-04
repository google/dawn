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
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_10;
void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  f0 = 1.0f;
  f1 = fract(f0);
  if ((tint_symbol.x > x_8.x_GLF_uniform_float_values[0].el)) {
    float v = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(f1);
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
