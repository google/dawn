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
  float x_67 = x_7.x_GLF_uniform_float_values[0].el;
  v[1u] = x_67;
  a = 2;
  float x_70 = x_7.x_GLF_uniform_float_values[1].el;
  float x_73 = x_7.x_GLF_uniform_float_values[1].el;
  vec2 x_75 = v;
  int x_77 = a;
  vec2 v_1 = vec2(x_70, x_70);
  indexable = vec2[3](v_1, vec2(x_73, x_73), x_75);
  vec2 x_79 = indexable[x_77];
  return x_79;
}
void main_1() {
  vec2 x_40 = func_();
  float x_43 = x_7.x_GLF_uniform_float_values[0].el;
  if ((x_40[1u] == x_43)) {
    int x_49 = x_11.x_GLF_uniform_int_values[0].el;
    int x_52 = x_11.x_GLF_uniform_int_values[1].el;
    int x_55 = x_11.x_GLF_uniform_int_values[1].el;
    int x_58 = x_11.x_GLF_uniform_int_values[0].el;
    float v_2 = float(x_49);
    float v_3 = float(x_52);
    float v_4 = float(x_55);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_58));
  } else {
    int x_62 = x_11.x_GLF_uniform_int_values[1].el;
    float x_63 = float(x_62);
    x_GLF_color = vec4(x_63, x_63, x_63, x_63);
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
