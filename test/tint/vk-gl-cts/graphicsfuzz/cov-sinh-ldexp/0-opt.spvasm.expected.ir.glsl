SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v1 = vec2(0.0f);
  ivec2 v2 = ivec2(0);
  vec2 v3 = vec2(0.0f);
  bool x_66 = false;
  bool x_67 = false;
  v1 = sinh(vec2(x_6.x_GLF_uniform_float_values[2].el, x_6.x_GLF_uniform_float_values[3].el));
  v2 = ivec2(x_9.x_GLF_uniform_int_values[0].el, -3000);
  v3 = ldexp(v1, v2);
  x_GLF_color = vec4(v3.y);
  bool x_59 = (v3.x > x_6.x_GLF_uniform_float_values[0].el);
  x_67 = x_59;
  if (x_59) {
    x_66 = (v3.x < x_6.x_GLF_uniform_float_values[1].el);
    x_67 = x_66;
  }
  if (x_67) {
    float v = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_9.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_9.x_GLF_uniform_int_values[0].el));
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
