#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
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
  float16_t x;
  float16_t tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  tint_pad16 tint_pad_11;
  f16vec3 y;
  float16_t tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  tint_pad16 tint_pad_15;
  tint_pad16 tint_pad_16;
  tint_pad16 tint_pad_17;
};

layout(binding = 0, std430)
buffer f_output_block_ssbo {
  S inner;
} v_1;
layout(location = 0) in float tint_interstage_location0;
layout(location = 1) flat in uint tint_interstage_location1;
layout(location = 2) in float16_t tint_interstage_location2;
layout(location = 3) in f16vec3 tint_interstage_location3;
void tint_store_and_preserve_padding(S value_param) {
  v_1.inner.f = value_param.f;
  v_1.inner.u = value_param.u;
  v_1.inner.v = value_param.v;
  v_1.inner.x = value_param.x;
  v_1.inner.y = value_param.y;
}
void frag_main_inner(S v_2) {
  float f = v_2.f;
  uint u = v_2.u;
  vec4 v = v_2.v;
  float16_t x = v_2.x;
  f16vec3 y = v_2.y;
  tint_store_and_preserve_padding(v_2);
}
void main() {
  frag_main_inner(S(tint_interstage_location0, tint_interstage_location1, 0u, 0u, tint_pad16_init, tint_pad16_init, tint_pad16_init, tint_pad64_init, gl_FragCoord, tint_pad16_init, tint_interstage_location2, 0.0hf, 0u, 0u, 0u, tint_pad16_init, tint_interstage_location3, 0.0hf, 0u, 0u, tint_pad16_init, tint_pad16_init, tint_pad16_init));
}
