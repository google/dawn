SKIP: FAILED

#version 310 es

struct buf1 {
  vec2 v1;
};

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


uniform buf1 x_6;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_8;
void main_1() {
  bool b = false;
  b = true;
  float x_38 = x_6.v1.x;
  float x_40 = x_6.v1.y;
  if ((x_38 > x_40)) {
    float x_45 = x_6.v1.x;
    float x_47 = x_6.v1.y;
    if ((x_45 < x_47)) {
      b = false;
    }
  }
  bool x_51 = b;
  if (x_51) {
    int x_10 = x_8.x_GLF_uniform_int_values[0].el;
    int x_11 = x_8.x_GLF_uniform_int_values[1].el;
    int x_12 = x_8.x_GLF_uniform_int_values[1].el;
    int x_13 = x_8.x_GLF_uniform_int_values[0].el;
    float v = float(x_10);
    float v_1 = float(x_11);
    float v_2 = float(x_12);
    x_GLF_color = vec4(v, v_1, v_2, float(x_13));
  } else {
    int x_14 = x_8.x_GLF_uniform_int_values[1].el;
    float x_65 = float(x_14);
    x_GLF_color = vec4(x_65, x_65, x_65, x_65);
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
