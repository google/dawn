//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow f_arg_0_arg_1;
float textureSampleCompareLevel_bcb3dd() {
  float res = textureGradOffset(f_arg_0_arg_1, vec4(vec2(1.0f), float(1u), 1.0f), vec2(0.0f), vec2(0.0f), ivec2(1));
  return res;
}
void main() {
  v.inner = textureSampleCompareLevel_bcb3dd();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleCompareLevel_bcb3dd() {
  float res = textureGradOffset(arg_0_arg_1, vec4(vec2(1.0f), float(1u), 1.0f), vec2(0.0f), vec2(0.0f), ivec2(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureSampleCompareLevel_bcb3dd();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DArrayShadow v_arg_0_arg_1;
layout(location = 0) flat out float tint_interstage_location0;
float textureSampleCompareLevel_bcb3dd() {
  float res = textureGradOffset(v_arg_0_arg_1, vec4(vec2(1.0f), float(1u), 1.0f), vec2(0.0f), vec2(0.0f), ivec2(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0.0f);
  v.pos = vec4(0.0f);
  v.prevent_dce = textureSampleCompareLevel_bcb3dd();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
