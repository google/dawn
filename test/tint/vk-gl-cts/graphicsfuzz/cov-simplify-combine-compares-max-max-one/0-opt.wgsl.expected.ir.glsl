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
  int a = 0;
  int x_24 = x_6.x_GLF_uniform_int_values[0].el;
  int x_26 = x_6.x_GLF_uniform_int_values[0].el;
  a = max(x_24, max(x_26, 1));
  int x_29 = a;
  int x_31 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_29 == x_31)) {
    int x_36 = a;
    int x_39 = x_6.x_GLF_uniform_int_values[1].el;
    int x_42 = x_6.x_GLF_uniform_int_values[1].el;
    int x_44 = a;
    float v = float(x_36);
    float v_1 = float(x_39);
    float v_2 = float(x_42);
    x_GLF_color = vec4(v, v_1, v_2, float(x_44));
  } else {
    int x_47 = a;
    float x_48 = float(x_47);
    x_GLF_color = vec4(x_48, x_48, x_48, x_48);
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
