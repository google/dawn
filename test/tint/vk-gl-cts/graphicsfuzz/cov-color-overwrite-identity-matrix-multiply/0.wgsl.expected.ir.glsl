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
  float x_33 = tint_symbol.x;
  float x_35 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_33 > x_35)) {
    float x_40 = x_6.x_GLF_uniform_float_values[2].el;
    x_GLF_color = vec4(x_40, x_40, x_40, x_40);
    float x_43 = tint_symbol.y;
    if ((x_43 > x_35)) {
      float x_48 = x_6.x_GLF_uniform_float_values[4].el;
      x_GLF_color = vec4(x_48, x_48, x_48, x_48);
    }
    float x_51 = x_6.x_GLF_uniform_float_values[3].el;
    x_GLF_color = vec4(x_51, x_51, x_51, x_51);
  }
  float x_54 = x_6.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4(x_35, x_54, x_54, 10.0f);
  vec4 x_61 = x_GLF_color;
  vec4 v = vec4(x_35, 0.0f, 0.0f, 0.0f);
  vec4 v_1 = vec4(0.0f, x_35, 0.0f, 0.0f);
  vec4 v_2 = vec4(0.0f, 0.0f, x_35, 0.0f);
  x_GLF_color = (mat4(v, v_1, v_2, vec4(0.0f, 0.0f, 0.0f, x_35)) * x_61);
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
