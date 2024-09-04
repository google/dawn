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
  vec4 x_GLF_v1_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_8;
uniform buf0 x_12;
vec4 x_GLF_v1 = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  vec2 uv = vec2(0.0f);
  vec4 v1 = vec4(0.0f);
  float a = 0.0f;
  int i = 0;
  vec4 x_49 = tint_symbol;
  uv = vec2(x_49[0u], x_49[1u]);
  float x_52 = x_8.x_GLF_uniform_float_values[0].el;
  v1 = vec4(x_52, x_52, x_52, x_52);
  float x_55 = uv.y;
  float x_57 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_55 >= x_57)) {
    float x_62 = x_8.x_GLF_uniform_float_values[2].el;
    v1[0u] = x_62;
    float x_65 = x_8.x_GLF_uniform_float_values[0].el;
    v1[1u] = x_65;
    float x_68 = x_8.x_GLF_uniform_float_values[0].el;
    v1[2u] = x_68;
    float x_71 = x_8.x_GLF_uniform_float_values[2].el;
    v1[3u] = x_71;
  }
  float x_74 = x_8.x_GLF_uniform_float_values[2].el;
  a = x_74;
  int x_15 = x_12.x_GLF_uniform_int_values[1].el;
  i = x_15;
  {
    while(true) {
      int x_16 = i;
      int x_17 = x_12.x_GLF_uniform_int_values[0].el;
      if ((x_16 < x_17)) {
      } else {
        break;
      }
      float x_84 = x_8.x_GLF_uniform_float_values[2].el;
      float x_86 = x_8.x_GLF_uniform_float_values[0].el;
      if ((x_84 < x_86)) {
        continue_execution = false;
      }
      float x_91 = v1.x;
      float x_93 = v1.y;
      float x_96 = v1.z;
      float x_99 = v1.w;
      float x_102 = x_8.x_GLF_uniform_float_values[3].el;
      a = pow((((x_91 + x_93) + x_96) + x_99), x_102);
      {
        int x_18 = i;
        i = (x_18 + 1);
      }
      continue;
    }
  }
  float x_104 = a;
  float x_106 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_104 == x_106)) {
    vec4 x_111 = v1;
    x_GLF_v1 = x_111;
  } else {
    int x_20 = x_12.x_GLF_uniform_int_values[1].el;
    float x_113 = float(x_20);
    x_GLF_v1 = vec4(x_113, x_113, x_113, x_113);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v = main_out(x_GLF_v1);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
