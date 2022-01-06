SKIP: FAILED

#version 310 es
precision mediump float;

uint tint_pack2x16unorm(vec2 param_0) {
  uint2 i = uint2(round(clamp(param_0, 0.0, 1.0) * 65535.0));
  return (i.x | i.y << 16);
}

vec4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}


struct tint_padded_array_element {
  float el;
};
struct buf1 {
  tint_padded_array_element x_GLF_uniform_float_values[7];
};
struct tint_padded_array_element_1 {
  int el;
};
struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_float_values[7];
} x_6;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_10;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  vec4 v1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float E = 0.0f;
  bool x_75 = false;
  bool x_92 = false;
  bool x_109 = false;
  bool x_76_phi = false;
  bool x_93_phi = false;
  bool x_110_phi = false;
  float x_41 = x_6.x_GLF_uniform_float_values[0].el;
  float x_43 = x_6.x_GLF_uniform_float_values[1].el;
  a = tint_pack2x16unorm(vec2(x_41, x_43));
  v1 = tint_unpack4x8snorm(a);
  E = 0.01f;
  int x_49 = x_10.x_GLF_uniform_int_values[2].el;
  float x_51 = v1[x_49];
  float x_53 = x_6.x_GLF_uniform_float_values[2].el;
  float x_55 = x_6.x_GLF_uniform_float_values[3].el;
  bool x_60 = (abs((x_51 - (x_53 / x_55))) < E);
  x_76_phi = x_60;
  if (x_60) {
    int x_64 = x_10.x_GLF_uniform_int_values[1].el;
    float x_66 = v1[x_64];
    float x_68 = x_6.x_GLF_uniform_float_values[4].el;
    float x_70 = x_6.x_GLF_uniform_float_values[3].el;
    x_75 = (abs((x_66 - (x_68 / x_70))) < E);
    x_76_phi = x_75;
  }
  bool x_76 = x_76_phi;
  x_93_phi = x_76;
  if (x_76) {
    int x_80 = x_10.x_GLF_uniform_int_values[3].el;
    float x_82 = v1[x_80];
    float x_84 = x_6.x_GLF_uniform_float_values[5].el;
    float x_87 = x_6.x_GLF_uniform_float_values[3].el;
    x_92 = (abs((x_82 - (-(x_84) / x_87))) < E);
    x_93_phi = x_92;
  }
  bool x_93 = x_93_phi;
  x_110_phi = x_93;
  if (x_93) {
    int x_97 = x_10.x_GLF_uniform_int_values[0].el;
    float x_99 = v1[x_97];
    float x_101 = x_6.x_GLF_uniform_float_values[6].el;
    float x_104 = x_6.x_GLF_uniform_float_values[3].el;
    x_109 = (abs((x_99 - (-(x_101) / x_104))) < E);
    x_110_phi = x_109;
  }
  if (x_110_phi) {
    int x_115 = x_10.x_GLF_uniform_int_values[1].el;
    int x_118 = x_10.x_GLF_uniform_int_values[2].el;
    int x_121 = x_10.x_GLF_uniform_int_values[2].el;
    int x_124 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_115), float(x_118), float(x_121), float(x_124));
  } else {
    float x_128 = x_6.x_GLF_uniform_float_values[5].el;
    x_GLF_color = vec4(x_128, x_128, x_128, x_128);
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



