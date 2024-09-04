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
  int x_29 = x_6.x_GLF_uniform_int_values[0].el;
  float x_30 = float(x_29);
  vec2 v = vec2(x_30, 0.0f);
  m = transpose(transpose(mat2(v, vec2(0.0f, x_30))));
  mat2 x_36 = m;
  int x_38 = x_6.x_GLF_uniform_int_values[0].el;
  float x_39 = float(x_38);
  vec2 v_1 = vec2(x_39, 0.0f);
  mat2 x_42 = mat2(v_1, vec2(0.0f, x_39));
  bool v_2 = all((x_36[0u] == x_42[0u]));
  if ((v_2 & all((x_36[1u] == x_42[1u])))) {
    int x_56 = x_6.x_GLF_uniform_int_values[0].el;
    int x_59 = x_6.x_GLF_uniform_int_values[1].el;
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    int x_65 = x_6.x_GLF_uniform_int_values[0].el;
    float v_3 = float(x_56);
    float v_4 = float(x_59);
    float v_5 = float(x_62);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_65));
  } else {
    int x_69 = x_6.x_GLF_uniform_int_values[1].el;
    float x_70 = float(x_69);
    x_GLF_color = vec4(x_70, x_70, x_70, x_70);
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
