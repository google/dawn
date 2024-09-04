SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2 m = mat2(vec2(0.0f), vec2(0.0f));
  float x_30 = float(x_6.x_GLF_uniform_int_values[0].el);
  vec2 v = vec2(x_30, 0.0f);
  m = transpose(transpose(mat2(v, vec2(0.0f, x_30))));
  float x_39 = float(x_6.x_GLF_uniform_int_values[0].el);
  vec2 v_1 = vec2(x_39, 0.0f);
  mat2 x_42 = mat2(v_1, vec2(0.0f, x_39));
  bool v_2 = all((m[0u] == x_42[0u]));
  if ((v_2 & all((m[1u] == x_42[1u])))) {
    float v_3 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_4 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_5 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_6.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
