#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
vec4 tint_quantize_to_f16(vec4 val) {
  vec2 v_1 = unpackHalf2x16(packHalf2x16(val.xy));
  return vec4(v_1, unpackHalf2x16(packHalf2x16(val.zw)));
}
vec4 quantizeToF16_cba294() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = tint_quantize_to_f16(arg_0);
  return res;
}
void main() {
  v.inner = quantizeToF16_cba294();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
vec4 tint_quantize_to_f16(vec4 val) {
  vec2 v_1 = unpackHalf2x16(packHalf2x16(val.xy));
  return vec4(v_1, unpackHalf2x16(packHalf2x16(val.zw)));
}
vec4 quantizeToF16_cba294() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = tint_quantize_to_f16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = quantizeToF16_cba294();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 tint_quantize_to_f16(vec4 val) {
  vec2 v = unpackHalf2x16(packHalf2x16(val.xy));
  return vec4(v, unpackHalf2x16(packHalf2x16(val.zw)));
}
vec4 quantizeToF16_cba294() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = tint_quantize_to_f16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = quantizeToF16_cba294();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
