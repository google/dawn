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
  uv = tint_symbol.xy;
  v1 = vec4(x_8.x_GLF_uniform_float_values[0].el);
  if ((uv.y >= x_8.x_GLF_uniform_float_values[0].el)) {
    v1[0u] = x_8.x_GLF_uniform_float_values[2].el;
    v1[1u] = x_8.x_GLF_uniform_float_values[0].el;
    v1[2u] = x_8.x_GLF_uniform_float_values[0].el;
    v1[3u] = x_8.x_GLF_uniform_float_values[2].el;
  }
  a = x_8.x_GLF_uniform_float_values[2].el;
  i = x_12.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_12.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      if ((x_8.x_GLF_uniform_float_values[2].el < x_8.x_GLF_uniform_float_values[0].el)) {
        continue_execution = false;
      }
      a = pow((((v1.x + v1.y) + v1.z) + v1.w), x_8.x_GLF_uniform_float_values[3].el);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((a == x_8.x_GLF_uniform_float_values[1].el)) {
    x_GLF_v1 = v1;
  } else {
    x_GLF_v1 = vec4(float(x_12.x_GLF_uniform_int_values[1].el));
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
