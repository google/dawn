#version 310 es


struct Uniforms {
  mat4 modelViewProjectionMatrix;
};

struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
layout(location = 0) in vec4 vtx_main_loc0_Input;
layout(location = 1) in vec4 vtx_main_loc1_Input;
layout(location = 0) out vec4 vtx_main_loc0_Output;
VertexOutput vtx_main_inner(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.Position = (v.inner.modelViewProjectionMatrix * tint_symbol.cur_position);
  tint_symbol_1.vtxFragColor = tint_symbol.color;
  return tint_symbol_1;
}
void main() {
  VertexOutput v_1 = vtx_main_inner(VertexInput(vtx_main_loc0_Input, vtx_main_loc1_Input));
  vtx_main_loc0_Output = v_1.vtxFragColor;
  gl_Position = v_1.Position;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 frag_main_loc0_Input;
layout(location = 0) out vec4 frag_main_loc0_Output;
vec4 frag_main_inner(vec4 fragColor) {
  return fragColor;
}
void main() {
  frag_main_loc0_Output = frag_main_inner(frag_main_loc0_Input);
}
