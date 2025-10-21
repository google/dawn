//
// vtx_main
//
#version 310 es


struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};

layout(binding = 0, std140)
uniform v_uniforms_block_ubo {
  uvec4 inner[4];
} v;
layout(location = 0) in vec4 vtx_main_loc0_Input;
layout(location = 1) in vec4 vtx_main_loc1_Input;
layout(location = 0) out vec4 tint_interstage_location0;
mat4 v_1(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
VertexOutput vtx_main_inner(VertexInput v_2) {
  VertexOutput v_3 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_3.Position = (v_1(0u) * v_2.cur_position);
  v_3.vtxFragColor = v_2.color;
  return v_3;
}
void main() {
  VertexOutput v_4 = vtx_main_inner(VertexInput(vtx_main_loc0_Input, vtx_main_loc1_Input));
  tint_interstage_location0 = v_4.vtxFragColor;
  gl_Position = vec4(v_4.Position.x, -(v_4.Position.y), ((2.0f * v_4.Position.z) - v_4.Position.w), v_4.Position.w);
  gl_PointSize = 1.0f;
}
//
// frag_main
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 0) out vec4 frag_main_loc0_Output;
vec4 frag_main_inner(vec4 fragColor) {
  return fragColor;
}
void main() {
  frag_main_loc0_Output = frag_main_inner(tint_interstage_location0);
}
