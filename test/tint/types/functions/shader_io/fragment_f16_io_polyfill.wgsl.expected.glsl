#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 1) in float16_t loc1_1;
layout(location = 2) in f16vec4 loc2_1;
layout(location = 1) out float16_t a_1;
layout(location = 2) out f16vec4 b_1;
struct Outputs {
  float16_t a;
  f16vec4 b;
};

Outputs frag_main(float16_t loc1, f16vec4 loc2) {
  Outputs tint_symbol = Outputs((loc1 * 2.0hf), (loc2 * 3.0hf));
  return tint_symbol;
}

void main() {
  Outputs inner_result = frag_main(loc1_1, loc2_1);
  a_1 = inner_result.a;
  b_1 = inner_result.b;
  return;
}
