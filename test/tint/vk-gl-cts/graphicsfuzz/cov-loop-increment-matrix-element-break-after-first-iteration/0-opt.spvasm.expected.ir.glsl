SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_7;
uniform buf0 x_10;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2x3 m23 = mat2x3(vec3(0.0f), vec3(0.0f));
  int i = 0;
  float x_46 = x_7.x_GLF_uniform_float_values[1].el;
  vec3 v = vec3(x_46, 0.0f, 0.0f);
  m23 = mat2x3(v, vec3(0.0f, x_46, 0.0f));
  i = 1;
  {
    while(true) {
      bool x_80 = false;
      bool x_81 = false;
      if ((i < x_10.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      int x_60 = x_10.x_GLF_uniform_int_values[0].el;
      int x_62 = x_10.x_GLF_uniform_int_values[2].el;
      m23[x_60][x_62] = (m23[x_60][x_62] + x_7.x_GLF_uniform_float_values[0].el);
      if ((tint_symbol.y < x_7.x_GLF_uniform_float_values[0].el)) {
      }
      x_81 = true;
      if (true) {
        x_80 = (tint_symbol.x < 0.0f);
        x_81 = x_80;
      }
      if (!(x_81)) {
        break;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float v_1 = float(x_10.x_GLF_uniform_int_values[1].el);
  float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
  vec3 v_3 = vec3(v_1, v_2, float(x_10.x_GLF_uniform_int_values[1].el));
  float v_4 = float(x_10.x_GLF_uniform_int_values[1].el);
  float v_5 = float(x_10.x_GLF_uniform_int_values[1].el);
  mat2x3 x_108 = mat2x3(v_3, vec3(v_4, v_5, float(x_10.x_GLF_uniform_int_values[0].el)));
  bool v_6 = all((m23[0u] == x_108[0u]));
  if ((v_6 & all((m23[1u] == x_108[1u])))) {
    float v_7 = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_8 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_9 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_7, v_8, v_9, float(x_10.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[1].el));
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
