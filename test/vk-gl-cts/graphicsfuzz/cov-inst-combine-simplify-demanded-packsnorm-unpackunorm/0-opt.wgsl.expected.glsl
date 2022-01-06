SKIP: FAILED

#version 310 es
precision mediump float;

uint tint_pack4x8snorm(vec4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


struct tint_padded_array_element {
  float el;
};
struct buf1 {
  tint_padded_array_element x_GLF_uniform_float_values[4];
};
struct tint_padded_array_element_1 {
  int el;
};
struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_float_values[4];
} x_6;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_10;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  vec4 v1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float E = 0.0f;
  bool x_69 = false;
  bool x_85 = false;
  bool x_101 = false;
  bool x_70_phi = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  float x_35 = x_6.x_GLF_uniform_float_values[1].el;
  a = tint_pack4x8snorm(vec4(x_35, x_35, x_35, x_35));
  v1 = tint_unpack4x8unorm(a);
  float x_41 = x_6.x_GLF_uniform_float_values[0].el;
  E = x_41;
  int x_43 = x_10.x_GLF_uniform_int_values[1].el;
  float x_45 = v1[x_43];
  float x_47 = x_6.x_GLF_uniform_float_values[2].el;
  float x_49 = x_6.x_GLF_uniform_float_values[3].el;
  bool x_54 = (abs((x_45 - (x_47 / x_49))) < E);
  x_70_phi = x_54;
  if (x_54) {
    int x_58 = x_10.x_GLF_uniform_int_values[0].el;
    float x_60 = v1[x_58];
    float x_62 = x_6.x_GLF_uniform_float_values[2].el;
    float x_64 = x_6.x_GLF_uniform_float_values[3].el;
    x_69 = (abs((x_60 - (x_62 / x_64))) < E);
    x_70_phi = x_69;
  }
  bool x_70 = x_70_phi;
  x_86_phi = x_70;
  if (x_70) {
    int x_74 = x_10.x_GLF_uniform_int_values[3].el;
    float x_76 = v1[x_74];
    float x_78 = x_6.x_GLF_uniform_float_values[2].el;
    float x_80 = x_6.x_GLF_uniform_float_values[3].el;
    x_85 = (abs((x_76 - (x_78 / x_80))) < E);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    int x_90 = x_10.x_GLF_uniform_int_values[2].el;
    float x_92 = v1[x_90];
    float x_94 = x_6.x_GLF_uniform_float_values[2].el;
    float x_96 = x_6.x_GLF_uniform_float_values[3].el;
    x_101 = (abs((x_92 - (x_94 / x_96))) < E);
    x_102_phi = x_101;
  }
  if (x_102_phi) {
    int x_107 = x_10.x_GLF_uniform_int_values[0].el;
    int x_110 = x_10.x_GLF_uniform_int_values[1].el;
    int x_113 = x_10.x_GLF_uniform_int_values[1].el;
    int x_116 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_107), float(x_110), float(x_113), float(x_116));
  } else {
    int x_120 = x_10.x_GLF_uniform_int_values[1].el;
    float x_121 = float(x_120);
    x_GLF_color = vec4(x_121, x_121, x_121, x_121);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:5: 'int4' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



