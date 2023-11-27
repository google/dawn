#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;

layout(location = 0) in float f_1;
layout(location = 1) flat in uint u_1;
layout(location = 2) in float16_t x_1;
layout(location = 3) in f16vec3 y_1;
struct S {
  float f;
  uint u;
  uint pad;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
  uint pad_22;
  uint pad_23;
  uint pad_24;
  uint pad_25;
  uint pad_26;
  uint pad_27;
  uint pad_28;
  uint pad_29;
  vec4 v;
  uint pad_30;
  uint pad_31;
  uint pad_32;
  uint pad_33;
  float16_t x;
  uint pad_34;
  uint pad_35;
  uint pad_36;
  uint pad_37;
  uint pad_38;
  uint pad_39;
  uint pad_40;
  f16vec3 y;
  uint pad_41;
  uint pad_42;
  uint pad_43;
  uint pad_44;
  uint pad_45;
  uint pad_46;
  uint pad_47;
  uint pad_48;
  uint pad_49;
  uint pad_50;
  uint pad_51;
  uint pad_52;
  uint pad_53;
  uint pad_54;
};

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  S inner;
} tint_symbol;

void assign_and_preserve_padding_tint_symbol(S value) {
  tint_symbol.inner.f = value.f;
  tint_symbol.inner.u = value.u;
  tint_symbol.inner.v = value.v;
  tint_symbol.inner.x = value.x;
  tint_symbol.inner.y = value.y;
}

void frag_main(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  float16_t x = tint_symbol_1.x;
  f16vec3 y = tint_symbol_1.y;
  assign_and_preserve_padding_tint_symbol(tint_symbol_1);
}

void main() {
  S tint_symbol_2 = S(f_1, u_1, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, gl_FragCoord, 0u, 0u, 0u, 0u, x_1, 0u, 0u, 0u, 0u, 0u, 0u, 0u, y_1, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
  frag_main(tint_symbol_2);
  return;
}
