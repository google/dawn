SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  int i = 0;
  float v = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[1].el);
  x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[0].el));
  if ((tint_symbol.y < 0.0f)) {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
  }
  i = x_6.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_6.x_GLF_uniform_int_values[2].el)) {
      } else {
        break;
      }
      if ((tint_symbol.x > 0.0f)) {
        if ((tint_symbol.y < 0.0f)) {
          x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
          break;
        }
      }
      if ((tint_symbol.x > 0.0f)) {
        if ((tint_symbol.y < 0.0f)) {
          x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
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
