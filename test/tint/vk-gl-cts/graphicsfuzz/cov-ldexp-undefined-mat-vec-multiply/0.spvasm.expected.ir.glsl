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


uniform buf0 x_6;
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v1 = vec2(0.0f);
  float x_35 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec2(x_35);
  int x_38 = x_8.x_GLF_uniform_int_values[0].el;
  v1[x_38] = ldexp(v1.y, -256);
  vec2 v = v1;
  vec2 v_1 = vec2(x_35, 0.0f);
  if (((v * mat2(v_1, vec2(0.0f, x_35)))[0u] == x_35)) {
    float x_53 = float(x_38);
    float x_56 = float(x_8.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(x_53, x_56, x_56, x_53);
  } else {
    x_GLF_color = vec4(float(x_8.x_GLF_uniform_int_values[1].el));
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
