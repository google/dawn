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
  int x_31 = x_6.x_GLF_uniform_int_values[0].el;
  int x_34 = x_6.x_GLF_uniform_int_values[1].el;
  int x_37 = x_6.x_GLF_uniform_int_values[1].el;
  int x_40 = x_6.x_GLF_uniform_int_values[0].el;
  float v = float(x_31);
  float v_1 = float(x_34);
  float v_2 = float(x_37);
  x_GLF_color = vec4(v, v_1, v_2, float(x_40));
  float x_44 = tint_symbol.y;
  if ((x_44 < 0.0f)) {
    int x_49 = x_6.x_GLF_uniform_int_values[1].el;
    float x_50 = float(x_49);
    x_GLF_color = vec4(x_50, x_50, x_50, x_50);
  }
  int x_53 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_53;
  {
    while(true) {
      int x_58 = i;
      int x_60 = x_6.x_GLF_uniform_int_values[2].el;
      if ((x_58 < x_60)) {
      } else {
        break;
      }
      float x_64 = tint_symbol.x;
      if ((x_64 > 0.0f)) {
        float x_69 = tint_symbol.y;
        if ((x_69 < 0.0f)) {
          int x_74 = x_6.x_GLF_uniform_int_values[1].el;
          float x_75 = float(x_74);
          x_GLF_color = vec4(x_75, x_75, x_75, x_75);
          break;
        }
      }
      float x_78 = tint_symbol.x;
      if ((x_78 > 0.0f)) {
        float x_83 = tint_symbol.y;
        if ((x_83 < 0.0f)) {
          int x_88 = x_6.x_GLF_uniform_int_values[1].el;
          float x_89 = float(x_88);
          x_GLF_color = vec4(x_89, x_89, x_89, x_89);
        }
      }
      {
        int x_91 = i;
        i = (x_91 + 1);
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
