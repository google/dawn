SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


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

layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  buf1 tint_symbol_3;
} v;
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  buf0 tint_symbol_5;
} v_1;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
void main_1() {
  mat2x3 m23 = mat2x3(vec3(0.0f), vec3(0.0f));
  int i = 0;
  float x_46 = v.tint_symbol_3.x_GLF_uniform_float_values[1].el;
  vec3 v_2 = vec3(x_46, 0.0f, 0.0f);
  m23 = mat2x3(v_2, vec3(0.0f, x_46, 0.0f));
  i = 1;
  {
    while(true) {
      bool x_80 = false;
      bool x_81 = false;
      if ((i < v_1.tint_symbol_5.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      int x_60 = v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el;
      int x_62 = v_1.tint_symbol_5.x_GLF_uniform_int_values[2].el;
      m23[x_60][x_62] = (m23[x_60][x_62] + v.tint_symbol_3.x_GLF_uniform_float_values[0].el);
      if ((tint_symbol.y < v.tint_symbol_3.x_GLF_uniform_float_values[0].el)) {
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
  float v_3 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el);
  float v_4 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el);
  vec3 v_5 = vec3(v_3, v_4, float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el));
  float v_6 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el);
  float v_7 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el);
  mat2x3 x_108 = mat2x3(v_5, vec3(v_6, v_7, float(v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el)));
  bool v_8 = all((m23[0u] == x_108[0u]));
  if ((v_8 & all((m23[1u] == x_108[1u])))) {
    float v_9 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el);
    float v_10 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el);
    float v_11 = float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_9, v_10, v_11, float(v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(v_1.tint_symbol_5.x_GLF_uniform_int_values[1].el));
  }
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:77: 'all' : no matching overloaded function found 
ERROR: 0:77: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:77: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
