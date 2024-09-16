SKIP: FAILED

#version 310 es


struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

layout(binding = 0, std140)
uniform tint_symbol_3_1_ubo {
  RenderParams tint_symbol_2;
} v;
layout(location = 0) in vec3 vs_main_loc0_Input;
layout(location = 1) in vec4 vs_main_loc1_Input;
layout(location = 2) in vec2 vs_main_loc2_Input;
layout(location = 0) out vec4 vs_main_loc0_Output;
layout(location = 1) out vec2 vs_main_loc1_Output;
VertexOutput vs_main_inner(VertexInput tint_symbol) {
  vec3 quad_pos = (mat2x3(v.tint_symbol_2.right, v.tint_symbol_2.up) * tint_symbol.quad_pos);
  vec3 position = (tint_symbol.position + (quad_pos * 0.00999999977648258209f));
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f), vec4(0.0f), vec2(0.0f));
  mat4 v_1 = v.tint_symbol_2.modelViewProjectionMatrix;
  tint_symbol_1.position = (v_1 * vec4(position, 1.0f));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}
void main() {
  VertexOutput v_2 = vs_main_inner(VertexInput(vs_main_loc0_Input, vs_main_loc1_Input, vs_main_loc2_Input));
  gl_Position = v_2.position;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vs_main_loc0_Output = v_2.color;
  vs_main_loc1_Output = v_2.quad_pos;
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

layout(location = 0) in vec4 fs_main_loc0_Input;
layout(location = 1) in vec2 fs_main_loc1_Input;
layout(location = 0) out vec4 fs_main_loc0_Output;
vec4 fs_main_inner(VertexOutput tint_symbol) {
  vec4 color = tint_symbol.color;
  float v = color.w;
  color[3u] = (v * max((1.0f - length(tint_symbol.quad_pos)), 0.0f));
  return color;
}
void main() {
  fs_main_loc0_Output = fs_main_inner(VertexOutput(gl_FragCoord, fs_main_loc0_Input, fs_main_loc1_Input));
}
<dawn>/src/tint/lang/glsl/writer/printer/printer.cc:1423 internal compiler error: TINT_UNREACHABLE unhandled core builtin: textureNumLevels
********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************

tint executable returned error: signal: trace/BPT trap
