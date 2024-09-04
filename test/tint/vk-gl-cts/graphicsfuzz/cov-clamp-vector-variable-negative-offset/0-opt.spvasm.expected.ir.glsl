SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
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
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_9;
void main_1() {
  vec2 v0 = vec2(0.0f);
  vec2 v1 = vec2(0.0f);
  v0 = vec2(x_6.x_GLF_uniform_float_values[2].el, 3.79999995231628417969f);
  vec2 v = (v0 - vec2(1.0f));
  v1 = clamp(v, vec2(0.0f), vec2(x_6.x_GLF_uniform_float_values[1].el));
  vec2 v_1 = v1;
  if (all((v_1 == vec2(x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el)))) {
    float v_2 = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_3 = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_4 = float(x_9.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_9.x_GLF_uniform_int_values[0].el));
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
