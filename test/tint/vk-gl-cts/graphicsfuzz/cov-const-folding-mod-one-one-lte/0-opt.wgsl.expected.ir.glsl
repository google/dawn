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
  if (true) {
    int x_29 = x_5.x_GLF_uniform_int_values[0].el;
    int x_32 = x_5.x_GLF_uniform_int_values[0].el;
    int x_35 = x_5.x_GLF_uniform_int_values[1].el;
    float v = float(x_29);
    float v_1 = float(x_32);
    x_GLF_color = vec4(1.0f, v, v_1, float(x_35));
  } else {
    int x_39 = x_5.x_GLF_uniform_int_values[0].el;
    float x_40 = float(x_39);
    x_GLF_color = vec4(x_40, x_40, x_40, x_40);
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
