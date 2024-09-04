SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_v1_1;
};
precision highp float;
precision highp int;


uniform buf1 x_7;
uniform buf0 x_9;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_v1 = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  int i = 0;
  int j = 0;
  i = x_7.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_7.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      if ((x_9.x_GLF_uniform_float_values[0].el > x_9.x_GLF_uniform_float_values[1].el)) {
        continue_execution = false;
      }
      j = x_7.x_GLF_uniform_int_values[1].el;
      {
        while(true) {
          if ((j < x_7.x_GLF_uniform_int_values[0].el)) {
          } else {
            break;
          }
          if ((tint_symbol.x < x_9.x_GLF_uniform_float_values[0].el)) {
            continue_execution = false;
          }
          float v = float(x_7.x_GLF_uniform_int_values[2].el);
          float v_1 = float(x_7.x_GLF_uniform_int_values[1].el);
          float v_2 = float(x_7.x_GLF_uniform_int_values[1].el);
          x_GLF_v1 = vec4(v, v_1, v_2, float(x_7.x_GLF_uniform_int_values[2].el));
          {
            j = (j + 1);
          }
          continue;
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
  main_out v_3 = main_out(x_GLF_v1);
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
