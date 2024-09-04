SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v1 = vec2(0.0f);
  vec2 b = vec2(0.0f);
  float a = 0.0f;
  bool x_51 = false;
  bool x_52 = false;
  v1 = vec2(x_6.x_GLF_uniform_float_values[0].el);
  b = fract(v1);
  a = smoothstep(vec2(1.0f), vec2(2.0f), b)[0u];
  x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el, a, a, x_6.x_GLF_uniform_float_values[0].el);
  bool x_46 = (b.x < 1.0f);
  x_52 = x_46;
  if (x_46) {
    x_51 = (b.y < 1.0f);
    x_52 = x_51;
  }
  if (x_52) {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el, b.x, b.y, x_6.x_GLF_uniform_float_values[0].el);
  } else {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el);
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
