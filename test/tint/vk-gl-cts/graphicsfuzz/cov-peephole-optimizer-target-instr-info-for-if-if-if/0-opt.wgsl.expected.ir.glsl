SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_9;
bool continue_execution = true;
void main_1() {
  int i = 0;
  int x_37 = x_6.x_GLF_uniform_int_values[1].el;
  float x_38 = float(x_37);
  x_GLF_color = vec4(x_38, x_38, x_38, x_38);
  int x_41 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_41;
  {
    while(true) {
      int x_46 = i;
      int x_48 = x_6.x_GLF_uniform_int_values[2].el;
      if ((x_46 < x_48)) {
      } else {
        break;
      }
      float x_52 = tint_symbol.y;
      float x_54 = x_9.x_GLF_uniform_float_values[0].el;
      if ((x_52 < x_54)) {
        float x_59 = tint_symbol.x;
        float x_61 = x_9.x_GLF_uniform_float_values[0].el;
        if ((x_59 < x_61)) {
          return;
        }
        float x_66 = x_9.x_GLF_uniform_float_values[1].el;
        float x_68 = x_9.x_GLF_uniform_float_values[1].el;
        if ((x_66 > x_68)) {
          return;
        }
        continue_execution = false;
      }
      float x_73 = x_9.x_GLF_uniform_float_values[1].el;
      float x_75 = x_9.x_GLF_uniform_float_values[0].el;
      if ((x_73 > x_75)) {
        int x_80 = x_6.x_GLF_uniform_int_values[0].el;
        int x_83 = x_6.x_GLF_uniform_int_values[1].el;
        int x_86 = x_6.x_GLF_uniform_int_values[1].el;
        int x_89 = x_6.x_GLF_uniform_int_values[0].el;
        float v = float(x_80);
        float v_1 = float(x_83);
        float v_2 = float(x_86);
        x_GLF_color = vec4(v, v_1, v_2, float(x_89));
        break;
      }
      float x_93 = x_9.x_GLF_uniform_float_values[0].el;
      if ((x_93 < 0.0f)) {
        continue_execution = false;
      }
      {
        int x_97 = i;
        i = (x_97 + 1);
      }
      continue;
    }
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_3 = main_out(x_GLF_color);
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
