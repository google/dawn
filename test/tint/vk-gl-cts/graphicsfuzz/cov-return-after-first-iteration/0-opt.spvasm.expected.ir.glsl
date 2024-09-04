SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct buf2 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_7;
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_9;
uniform buf2 x_11;
void main_1() {
  int i = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  i = x_7.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_7.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      if ((i != x_7.x_GLF_uniform_int_values[1].el)) {
        return;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((tint_symbol.y < x_9.x_GLF_uniform_float_values[0].el)) {
    return;
  }
  x_GLF_color = vec4(1.0f, 1.0f, 1.0f, x_11.injectionSwitch.y);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
