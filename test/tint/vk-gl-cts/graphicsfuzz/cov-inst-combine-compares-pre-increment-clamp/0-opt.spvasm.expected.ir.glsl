SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[3];
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
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_14;
float f1_f1_(inout float a) {
  int b = 0;
  float c = 0.0f;
  b = 8;
  if ((tint_symbol.y >= x_9.x_GLF_uniform_float_values[0].el)) {
    b = (b + 1);
    b = (b + 1);
  }
  if ((a < x_9.x_GLF_uniform_float_values[1].el)) {
    float x_88 = x_9.x_GLF_uniform_float_values[1].el;
    return x_88;
  }
  c = float(min(max(b, 0), 2));
  float x_92 = c;
  return x_92;
}
void main_1() {
  float a_1 = 0.0f;
  float param = 0.0f;
  param = x_9.x_GLF_uniform_float_values[1].el;
  float x_44 = f1_f1_(param);
  a_1 = x_44;
  if ((a_1 == x_9.x_GLF_uniform_float_values[2].el)) {
    float v = float(x_14.x_GLF_uniform_int_values[1].el);
    float v_1 = float(x_14.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_14.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_14.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_14.x_GLF_uniform_int_values[0].el));
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
