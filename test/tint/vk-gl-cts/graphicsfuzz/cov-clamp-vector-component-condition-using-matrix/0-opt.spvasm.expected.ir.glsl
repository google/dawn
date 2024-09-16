SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v_1;
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_2;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec4 v = vec4(0.0f);
  int i = 0;
  float v_3 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el);
  float v_4 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[2].el);
  float v_5 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el);
  v = vec4(v_3, v_4, v_5, float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el));
  i = v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el;
  {
    while(true) {
      if ((i < v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      vec4 v_6 = vec4(v.x, v.y, v.z, v.w);
      vec4 v_7 = vec4(v.x, v.y, v.z, v.w);
      vec4 v_8 = vec4(v.x, v.y, v.z, v.w);
      mat4 v_9 = mat4(v_6, v_7, v_8, vec4(v.x, v.y, v.z, v.w));
      if ((v_9[0u][i] > v_2.tint_symbol_3.x_GLF_uniform_float_values[0].el)) {
        int x_96 = i;
        vec4 v_10 = v;
        vec4 v_11 = vec4(v_2.tint_symbol_3.x_GLF_uniform_float_values[1].el);
        vec4 v_12 = clamp(v_10, v_11, vec4(v_2.tint_symbol_3.x_GLF_uniform_float_values[0].el));
        v[x_96] = v_12[v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el];
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  vec4 v_13 = v;
  if (all((v_13 == vec4(float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el))))) {
    float v_14 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_15 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el);
    float v_16 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el);
    x_GLF_color = vec4(v_14, v_15, v_16, float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el));
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:68: 'all' : no matching overloaded function found 
ERROR: 0:68: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
