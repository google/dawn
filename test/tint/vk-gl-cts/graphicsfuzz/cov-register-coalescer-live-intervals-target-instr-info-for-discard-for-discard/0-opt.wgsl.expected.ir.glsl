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
  int x_36 = x_7.x_GLF_uniform_int_values[1].el;
  i = x_36;
  {
    while(true) {
      int x_41 = i;
      int x_43 = x_7.x_GLF_uniform_int_values[0].el;
      if ((x_41 < x_43)) {
      } else {
        break;
      }
      float x_47 = x_9.x_GLF_uniform_float_values[0].el;
      float x_49 = x_9.x_GLF_uniform_float_values[1].el;
      if ((x_47 > x_49)) {
        continue_execution = false;
      }
      int x_54 = x_7.x_GLF_uniform_int_values[1].el;
      j = x_54;
      {
        while(true) {
          int x_59 = j;
          int x_61 = x_7.x_GLF_uniform_int_values[0].el;
          if ((x_59 < x_61)) {
          } else {
            break;
          }
          float x_65 = tint_symbol.x;
          float x_67 = x_9.x_GLF_uniform_float_values[0].el;
          if ((x_65 < x_67)) {
            continue_execution = false;
          }
          int x_72 = x_7.x_GLF_uniform_int_values[2].el;
          int x_75 = x_7.x_GLF_uniform_int_values[1].el;
          int x_78 = x_7.x_GLF_uniform_int_values[1].el;
          int x_81 = x_7.x_GLF_uniform_int_values[2].el;
          float v = float(x_72);
          float v_1 = float(x_75);
          float v_2 = float(x_78);
          x_GLF_v1 = vec4(v, v_1, v_2, float(x_81));
          {
            int x_84 = j;
            j = (x_84 + 1);
          }
          continue;
        }
      }
      {
        int x_86 = i;
        i = (x_86 + 1);
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
