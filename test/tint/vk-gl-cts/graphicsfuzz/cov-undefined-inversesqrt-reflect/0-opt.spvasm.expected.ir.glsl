SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct buf1 {
  vec2 v1;
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
  mat2 m24 = mat2(vec2(0.0f), vec2(0.0f));
  float a = 0.0f;
  vec2 v2 = vec2(0.0f);
  vec2 v3 = vec2(0.0f);
  vec2 v = vec2(x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el);
  m24 = mat2(v, vec2((x_8.v1.x * 1.0f), x_6.x_GLF_uniform_float_values[0].el));
  a = m24[0u].x;
  v2 = vec2(1.0f);
  vec2 v_1 = v2;
  v3 = reflect(v_1, vec2(a, 1.0f));
  x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el, v3.x, v3.y, x_6.x_GLF_uniform_float_values[0].el);
  if ((x_8.v1.y == x_6.x_GLF_uniform_float_values[0].el)) {
    x_GLF_color = vec4(x_GLF_color.x, vec2(0.0f), x_GLF_color.w);
  } else {
    x_GLF_color = vec4(0.0f);
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
