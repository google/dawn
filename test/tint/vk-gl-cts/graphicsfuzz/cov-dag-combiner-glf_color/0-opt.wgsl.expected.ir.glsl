SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


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
  vec4 x_GLF_color_1;
};

vec4 x_GLF_color = vec4(0.0f);
layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v;
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
float func_f1_(inout float b) {
  float x_90 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  float x_92 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  float x_94 = v.tint_symbol_1.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4(x_90, x_92, x_94, 1.0f);
  vec4 x_96 = x_GLF_color;
  x_GLF_color = x_96;
  float x_98 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  float x_99 = b;
  if ((x_98 >= x_99)) {
    float x_104 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
    return x_104;
  }
  float x_106 = v.tint_symbol_1.x_GLF_uniform_float_values[2].el;
  return x_106;
}
void main_1() {
  float a = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  bool x_71 = false;
  bool x_72_phi = false;
  float x_44 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  param = x_44;
  float x_45 = func_f1_(param);
  a = x_45;
  float x_47 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  float x_49 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  param_1 = (x_47 + x_49);
  float x_51 = func_f1_(param_1);
  float x_52 = a;
  a = (x_52 + x_51);
  float x_54 = a;
  float x_56 = v.tint_symbol_1.x_GLF_uniform_float_values[3].el;
  bool x_57 = (x_54 == x_56);
  x_72_phi = x_57;
  if (x_57) {
    vec4 x_60 = x_GLF_color;
    float x_62 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
    float x_64 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
    float x_66 = v.tint_symbol_1.x_GLF_uniform_float_values[1].el;
    float x_68 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
    x_71 = all((x_60 == vec4(x_62, x_64, x_66, x_68)));
    x_72_phi = x_71;
  }
  bool x_72 = x_72_phi;
  if (x_72) {
    int x_15 = v_1.tint_symbol_3.x_GLF_uniform_int_values[0].el;
    int x_16 = v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    int x_17 = v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    int x_18 = v_1.tint_symbol_3.x_GLF_uniform_int_values[0].el;
    float v_2 = float(x_15);
    float v_3 = float(x_16);
    float v_4 = float(x_17);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_18));
  } else {
    int x_19 = v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    float x_86 = float(x_19);
    x_GLF_color = vec4(x_86, x_86, x_86, x_86);
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
ERROR: 0:78: 'all' : no matching overloaded function found 
ERROR: 0:78: 'assign' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:78: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
