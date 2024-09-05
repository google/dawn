#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct S {
  float f;
  uint u;
  vec4 v;
  float16_t x;
  f16vec3 y;
};

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  S tint_symbol_2;
} v_1;
layout(location = 0) in float frag_main_loc0_Input;
layout(location = 1) flat in uint frag_main_loc1_Input;
layout(location = 2) in float16_t frag_main_loc2_Input;
layout(location = 3) in f16vec3 frag_main_loc3_Input;
void tint_store_and_preserve_padding(inout S target, S value_param) {
  target.f = value_param.f;
  target.u = value_param.u;
  target.v = value_param.v;
  target.x = value_param.x;
  target.y = value_param.y;
}
void frag_main_inner(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  float16_t x = tint_symbol_1.x;
  f16vec3 y = tint_symbol_1.y;
  tint_store_and_preserve_padding(v_1.tint_symbol_2, tint_symbol_1);
}
void main() {
  frag_main_inner(S(frag_main_loc0_Input, frag_main_loc1_Input, gl_FragCoord, frag_main_loc2_Input, frag_main_loc3_Input));
}
