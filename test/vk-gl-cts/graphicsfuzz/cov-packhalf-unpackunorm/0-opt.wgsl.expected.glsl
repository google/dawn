SKIP: FAILED

#version 310 es
precision mediump float;

uint tint_pack2x16float(vec2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


struct tint_padded_array_element {
  float el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[4];
};
struct tint_padded_array_element_1 {
  int el;
};
struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[4];
} x_8;
layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_10;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  vec4 values = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 ref = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_85 = false;
  bool x_101 = false;
  bool x_117 = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  bool x_118_phi = false;
  a = tint_pack2x16float(vec2(1.0f, 1.0f));
  values = tint_unpack4x8unorm(a);
  float x_41 = x_8.x_GLF_uniform_float_values[3].el;
  float x_43 = x_8.x_GLF_uniform_float_values[1].el;
  float x_45 = x_8.x_GLF_uniform_float_values[0].el;
  float x_48 = x_8.x_GLF_uniform_float_values[3].el;
  float x_50 = x_8.x_GLF_uniform_float_values[0].el;
  float x_53 = x_8.x_GLF_uniform_float_values[1].el;
  float x_55 = x_8.x_GLF_uniform_float_values[0].el;
  ref = vec4(x_41, (x_43 / x_45), (x_48 / x_50), (x_53 / x_55));
  int x_59 = x_10.x_GLF_uniform_int_values[0].el;
  float x_61 = values[x_59];
  int x_63 = x_10.x_GLF_uniform_int_values[0].el;
  float x_65 = ref[x_63];
  float x_69 = x_8.x_GLF_uniform_float_values[2].el;
  bool x_70 = (abs((x_61 - x_65)) < x_69);
  x_86_phi = x_70;
  if (x_70) {
    int x_74 = x_10.x_GLF_uniform_int_values[1].el;
    float x_76 = values[x_74];
    int x_78 = x_10.x_GLF_uniform_int_values[1].el;
    float x_80 = ref[x_78];
    float x_84 = x_8.x_GLF_uniform_float_values[2].el;
    x_85 = (abs((x_76 - x_80)) < x_84);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    int x_90 = x_10.x_GLF_uniform_int_values[3].el;
    float x_92 = values[x_90];
    int x_94 = x_10.x_GLF_uniform_int_values[3].el;
    float x_96 = ref[x_94];
    float x_100 = x_8.x_GLF_uniform_float_values[2].el;
    x_101 = (abs((x_92 - x_96)) < x_100);
    x_102_phi = x_101;
  }
  bool x_102 = x_102_phi;
  x_118_phi = x_102;
  if (x_102) {
    int x_106 = x_10.x_GLF_uniform_int_values[2].el;
    float x_108 = values[x_106];
    int x_110 = x_10.x_GLF_uniform_int_values[2].el;
    float x_112 = ref[x_110];
    float x_116 = x_8.x_GLF_uniform_float_values[2].el;
    x_117 = (abs((x_108 - x_112)) < x_116);
    x_118_phi = x_117;
  }
  if (x_118_phi) {
    int x_123 = x_10.x_GLF_uniform_int_values[1].el;
    int x_126 = x_10.x_GLF_uniform_int_values[0].el;
    int x_129 = x_10.x_GLF_uniform_int_values[0].el;
    int x_132 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_123), float(x_126), float(x_129), float(x_132));
  } else {
    int x_136 = x_10.x_GLF_uniform_int_values[0].el;
    float x_137 = float(x_136);
    x_GLF_color = vec4(x_137, x_137, x_137, x_137);
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
ERROR: 0:5: 'uint2' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



