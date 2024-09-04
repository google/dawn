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


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
void main_1() {
  vec4 v = vec4(0.0f);
  int x_25 = x_5.x_GLF_uniform_int_values[0].el;
  int x_28 = x_5.x_GLF_uniform_int_values[1].el;
  int x_31 = x_5.x_GLF_uniform_int_values[1].el;
  int x_34 = x_5.x_GLF_uniform_int_values[0].el;
  float v_1 = float(x_25);
  float v_2 = float(x_28);
  float v_3 = float(x_31);
  x_GLF_color = vec4(v_1, v_2, v_3, float(x_34));
  vec4 x_37 = x_GLF_color;
  v = x_37;
  x_GLF_color = vec4(0.0f);
  vec4 x_38 = v;
  x_GLF_color = x_38;
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
