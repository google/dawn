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


vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_11;
float func_f1_(inout float x) {
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      {
        while(true) {
          if ((tint_symbol.y < x_8.x_GLF_uniform_float_values[2].el)) {
            {
              while(true) {
                {
                  float x_88 = tint_symbol.x;
                  float x_90 = x_8.x_GLF_uniform_float_values[2].el;
                  if (!((x_88 < x_90))) { break; }
                }
                continue;
              }
            }
          }
          if ((x < x_8.x_GLF_uniform_float_values[3].el)) {
            float x_99 = x_8.x_GLF_uniform_float_values[1].el;
            return x_99;
          }
          {
            float x_101 = tint_symbol.y;
            float x_103 = x_8.x_GLF_uniform_float_values[2].el;
            if (!((x_101 < x_103))) { break; }
          }
          continue;
        }
      }
      {
      }
      continue;
    }
  }
  float x_106 = x_8.x_GLF_uniform_float_values[0].el;
  return x_106;
}
void main_1() {
  float param = 0.0f;
  param = tint_symbol.x;
  float x_42 = func_f1_(param);
  if ((x_42 == x_8.x_GLF_uniform_float_values[1].el)) {
    float v = float(x_11.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_11.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_11.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_11.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_11.x_GLF_uniform_int_values[1].el));
  }
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
