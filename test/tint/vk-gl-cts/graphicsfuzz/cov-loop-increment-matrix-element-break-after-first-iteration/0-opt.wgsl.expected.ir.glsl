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
      bool x_81_phi = false;
      int x_54 = i;
      int x_56 = x_10.x_GLF_uniform_int_values[3].el;
      if ((x_54 < x_56)) {
      } else {
        break;
      }
      int x_60 = x_10.x_GLF_uniform_int_values[0].el;
      int x_62 = x_10.x_GLF_uniform_int_values[2].el;
      float x_64 = x_7.x_GLF_uniform_float_values[0].el;
      float x_66 = m23[x_60][x_62];
      m23[x_60][x_62] = (x_66 + x_64);
      float x_70 = tint_symbol.y;
      float x_72 = x_7.x_GLF_uniform_float_values[0].el;
      if ((x_70 < x_72)) {
      }
      x_81_phi = true;
      if (true) {
        float x_79 = tint_symbol.x;
        x_80 = (x_79 < 0.0f);
        x_81_phi = x_80;
      }
      bool x_81 = x_81_phi;
      if (!(x_81)) {
        break;
      }
      {
        int x_85 = i;
        i = (x_85 + 1);
      }
      continue;
    }
  }
  mat2x3 x_87 = m23;
  int x_89 = x_10.x_GLF_uniform_int_values[1].el;
  int x_92 = x_10.x_GLF_uniform_int_values[1].el;
  int x_95 = x_10.x_GLF_uniform_int_values[1].el;
  int x_98 = x_10.x_GLF_uniform_int_values[1].el;
  int x_101 = x_10.x_GLF_uniform_int_values[1].el;
  int x_104 = x_10.x_GLF_uniform_int_values[0].el;
  float v_1 = float(x_89);
  float v_2 = float(x_92);
  vec3 v_3 = vec3(v_1, v_2, float(x_95));
  float v_4 = float(x_98);
  float v_5 = float(x_101);
  mat2x3 x_108 = mat2x3(v_3, vec3(v_4, v_5, float(x_104)));
  bool v_6 = all((x_87[0u] == x_108[0u]));
  if ((v_6 & all((x_87[1u] == x_108[1u])))) {
    int x_122 = x_10.x_GLF_uniform_int_values[0].el;
    int x_125 = x_10.x_GLF_uniform_int_values[1].el;
    int x_128 = x_10.x_GLF_uniform_int_values[1].el;
    int x_131 = x_10.x_GLF_uniform_int_values[0].el;
    float v_7 = float(x_122);
    float v_8 = float(x_125);
    float v_9 = float(x_128);
    x_GLF_color = vec4(v_7, v_8, v_9, float(x_131));
  } else {
    int x_135 = x_10.x_GLF_uniform_int_values[1].el;
    float x_136 = float(x_135);
    x_GLF_color = vec4(x_136, x_136, x_136, x_136);
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
