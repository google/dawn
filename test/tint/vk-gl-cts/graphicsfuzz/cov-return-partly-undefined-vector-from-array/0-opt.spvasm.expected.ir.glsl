SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_11;
vec2 func_() {
  vec2 v = vec2(0.0f);
  int a = 0;
  vec2 indexable[3] = vec2[3](vec2(0.0f), vec2(0.0f), vec2(0.0f));
  v[1u] = x_7.x_GLF_uniform_float_values[0].el;
  a = 2;
  int x_77 = a;
  vec2 v_1 = vec2(x_7.x_GLF_uniform_float_values[1].el);
  vec2 v_2 = vec2(x_7.x_GLF_uniform_float_values[1].el);
  indexable = vec2[3](v_1, v_2, v);
  vec2 x_79 = indexable[x_77];
  return x_79;
}
void main_1() {
  vec2 x_40 = func_();
  if ((x_40[1u] == x_7.x_GLF_uniform_float_values[0].el)) {
    float v_3 = float(x_11.x_GLF_uniform_int_values[0].el);
    float v_4 = float(x_11.x_GLF_uniform_int_values[1].el);
    float v_5 = float(x_11.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_11.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_11.x_GLF_uniform_int_values[1].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
