#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct Outputs {
  float16_t a;
  f16vec4 b;
};

layout(location = 1) in float16_t frag_main_loc1_Input;
layout(location = 2) in f16vec4 frag_main_loc2_Input;
layout(location = 1) out float16_t frag_main_loc1_Output;
layout(location = 2) out f16vec4 frag_main_loc2_Output;
Outputs frag_main_inner(float16_t loc1, f16vec4 loc2) {
  return Outputs((loc1 * 2.0hf), (loc2 * 3.0hf));
}
void main() {
  Outputs v = frag_main_inner(frag_main_loc1_Input, frag_main_loc2_Input);
  frag_main_loc1_Output = v.a;
  frag_main_loc2_Output = v.b;
}
