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
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
int f1_vf2_(inout vec2 v1) {
  bool x_99 = false;
  bool x_100 = false;
  bool x_92 = (v1.x == x_7.x_GLF_uniform_float_values[0].el);
  x_100 = x_92;
  if (x_92) {
    x_99 = (v1.y == x_7.x_GLF_uniform_float_values[1].el);
    x_100 = x_99;
  }
  if (x_100) {
    int x_104 = x_9.x_GLF_uniform_int_values[1].el;
    return x_104;
  }
  int x_106 = x_9.x_GLF_uniform_int_values[0].el;
  return x_106;
}
void main_1() {
  mat2 m1 = mat2(vec2(0.0f), vec2(0.0f));
  mat2 m2 = mat2(vec2(0.0f), vec2(0.0f));
  vec2 v1_1 = vec2(0.0f);
  int a = 0;
  vec2 param = vec2(0.0f);
  vec2 v = vec2(x_7.x_GLF_uniform_float_values[0].el, -(x_7.x_GLF_uniform_float_values[1].el));
  float v_1 = x_7.x_GLF_uniform_float_values[1].el;
  m1 = mat2(v, vec2(v_1, sin(x_7.x_GLF_uniform_float_values[1].el)));
  m2 = (m1 * m1);
  vec2 v_2 = vec2(x_7.x_GLF_uniform_float_values[0].el);
  v1_1 = (v_2 * m2);
  param = v1_1;
  int x_66 = f1_vf2_(param);
  a = x_66;
  if ((a == x_9.x_GLF_uniform_int_values[1].el)) {
    x_GLF_color = vec4(x_7.x_GLF_uniform_float_values[0].el, x_7.x_GLF_uniform_float_values[1].el, x_7.x_GLF_uniform_float_values[1].el, x_7.x_GLF_uniform_float_values[0].el);
  } else {
    x_GLF_color = vec4(float(x_9.x_GLF_uniform_int_values[1].el));
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
