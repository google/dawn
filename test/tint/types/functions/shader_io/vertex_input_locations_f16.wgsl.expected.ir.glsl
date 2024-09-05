#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(location = 0) in int tint_symbol_loc0_Input;
layout(location = 1) in uint tint_symbol_loc1_Input;
layout(location = 2) in float tint_symbol_loc2_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
layout(location = 4) in float16_t tint_symbol_loc4_Input;
layout(location = 5) in f16vec3 tint_symbol_loc5_Input;
vec4 tint_symbol_inner(int loc0, uint loc1, float loc2, vec4 loc3, float16_t loc4, f16vec3 loc5) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
  float16_t x = loc4;
  f16vec3 y = loc5;
  return vec4(0.0f);
}
void main() {
  gl_Position = tint_symbol_inner(tint_symbol_loc0_Input, tint_symbol_loc1_Input, tint_symbol_loc2_Input, tint_symbol_loc3_Input, tint_symbol_loc4_Input, tint_symbol_loc5_Input);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
