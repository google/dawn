#version 310 es
precision highp float;
precision highp int;


struct tint_pad16 {
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
};
const tint_pad16 tint_pad16_init = tint_pad16(0u, 0u, 0u, 0u);

struct tint_pad64 {
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
};
const tint_pad64 tint_pad64_init = tint_pad64(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);

struct S {
  float f;
  uint u;
  uint tint_pad_0;
  uint tint_pad_1;
  tint_pad16 tint_pad_2;
  tint_pad16 tint_pad_3;
  tint_pad16 tint_pad_4;
  tint_pad64 tint_pad_5;
  vec4 v;
  tint_pad16 tint_pad_6;
  tint_pad16 tint_pad_7;
  tint_pad16 tint_pad_8;
  tint_pad64 tint_pad_9;
};

layout(binding = 0, std430)
buffer f_output_block_ssbo {
  S inner;
} v_1;
void tint_store_and_preserve_padding(S value_param) {
  v_1.inner.f = value_param.f;
  v_1.inner.u = value_param.u;
  v_1.inner.v = value_param.v;
}
void main() {
  tint_store_and_preserve_padding(S(1.0f, 2u, 0u, 0u, tint_pad16_init, tint_pad16_init, tint_pad16_init, tint_pad64_init, vec4(3.0f), tint_pad16_init, tint_pad16_init, tint_pad16_init, tint_pad64_init));
}
