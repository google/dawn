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
  bool x_100_phi = false;
  float x_89 = v1.x;
  float x_91 = x_7.x_GLF_uniform_float_values[0].el;
  bool x_92 = (x_89 == x_91);
  x_100_phi = x_92;
  if (x_92) {
    float x_96 = v1.y;
    float x_98 = x_7.x_GLF_uniform_float_values[1].el;
    x_99 = (x_96 == x_98);
    x_100_phi = x_99;
  }
  bool x_100 = x_100_phi;
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
  float x_45 = x_7.x_GLF_uniform_float_values[0].el;
  float x_47 = x_7.x_GLF_uniform_float_values[1].el;
  float x_50 = x_7.x_GLF_uniform_float_values[1].el;
  float x_52 = x_7.x_GLF_uniform_float_values[1].el;
  vec2 v = vec2(x_45, -(x_47));
  m1 = mat2(v, vec2(x_50, sin(x_52)));
  mat2 x_57 = m1;
  mat2 x_58 = m1;
  m2 = (x_57 * x_58);
  float x_61 = x_7.x_GLF_uniform_float_values[0].el;
  mat2 x_63 = m2;
  v1_1 = (vec2(x_61, x_61) * x_63);
  vec2 x_65 = v1_1;
  param = x_65;
  int x_66 = f1_vf2_(param);
  a = x_66;
  int x_67 = a;
  int x_69 = x_9.x_GLF_uniform_int_values[1].el;
  if ((x_67 == x_69)) {
    float x_75 = x_7.x_GLF_uniform_float_values[0].el;
    float x_77 = x_7.x_GLF_uniform_float_values[1].el;
    float x_79 = x_7.x_GLF_uniform_float_values[1].el;
    float x_81 = x_7.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_75, x_77, x_79, x_81);
  } else {
    int x_84 = x_9.x_GLF_uniform_int_values[1].el;
    float x_85 = float(x_84);
    x_GLF_color = vec4(x_85, x_85, x_85, x_85);
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
