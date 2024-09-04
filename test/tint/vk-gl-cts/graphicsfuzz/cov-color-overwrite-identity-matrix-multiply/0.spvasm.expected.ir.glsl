SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[5];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float x_35 = x_6.x_GLF_uniform_float_values[0].el;
  if ((tint_symbol.x > x_35)) {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[2].el);
    if ((tint_symbol.y > x_35)) {
      x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[4].el);
    }
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[3].el);
  }
  float x_54 = x_6.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4(x_35, x_54, x_54, 10.0f);
  vec4 v = vec4(x_35, 0.0f, 0.0f, 0.0f);
  vec4 v_1 = vec4(0.0f, x_35, 0.0f, 0.0f);
  vec4 v_2 = vec4(0.0f, 0.0f, x_35, 0.0f);
  mat4 v_3 = mat4(v, v_1, v_2, vec4(0.0f, 0.0f, 0.0f, x_35));
  x_GLF_color = (v_3 * x_GLF_color);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
