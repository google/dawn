SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct buf2 {
  float zero;
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_8;
uniform buf2 x_10;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_13;
bool continue_execution = true;
bool func_vf2_(inout vec2 pos) {
  float x_62 = pos.x;
  float x_64 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_62 < x_64)) {
    return true;
  }
  float x_69 = pos.y;
  float x_71 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_69 > x_71)) {
    return false;
  }
  float x_76 = x_10.zero;
  float x_78 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_76 > x_78)) {
    return true;
  }
  return true;
}
void main_1() {
  vec2 param = vec2(0.0f);
  vec4 x_42 = tint_symbol;
  param = vec2(x_42[0u], x_42[1u]);
  bool x_44 = func_vf2_(param);
  if (x_44) {
    continue_execution = false;
  }
  int x_48 = x_13.x_GLF_uniform_int_values[0].el;
  int x_51 = x_13.x_GLF_uniform_int_values[1].el;
  int x_54 = x_13.x_GLF_uniform_int_values[1].el;
  int x_57 = x_13.x_GLF_uniform_int_values[0].el;
  float v = float(x_48);
  float v_1 = float(x_51);
  float v_2 = float(x_54);
  x_GLF_color = vec4(v, v_1, v_2, float(x_57));
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_3 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
