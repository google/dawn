SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec2 v0 = vec2(0.0f);
  vec2 v1 = vec2(0.0f);
  float x_37 = v.tint_symbol_1.x_GLF_uniform_float_values[2].el;
  v0 = vec2(x_37, 3.79999995231628417969f);
  vec2 x_39 = v0;
  float x_43 = v.tint_symbol_1.x_GLF_uniform_float_values[1].el;
  v1 = clamp((x_39 - vec2(1.0f)), vec2(0.0f), vec2(x_43, x_43));
  vec2 x_47 = v1;
  float x_49 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  float x_51 = v.tint_symbol_1.x_GLF_uniform_float_values[1].el;
  if (all((x_47 == vec2(x_49, x_51)))) {
    int x_59 = v_1.tint_symbol_3.x_GLF_uniform_int_values[0].el;
    int x_62 = v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    int x_65 = v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    int x_68 = v_1.tint_symbol_3.x_GLF_uniform_int_values[0].el;
    float v_2 = float(x_59);
    float v_3 = float(x_62);
    float v_4 = float(x_65);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_68));
  } else {
    int x_72 = v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    float x_73 = float(x_72);
    x_GLF_color = vec4(x_73, x_73, x_73, x_73);
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
ERROR: 0:47: 'all' : no matching overloaded function found 
ERROR: 0:47: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
