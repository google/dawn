#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec3 tint_symbol;
} v;
vec3 tint_quantize_to_f16(vec3 val) {
  vec2 v_1 = unpackHalf2x16(packHalf2x16(val.xy));
  return vec3(v_1, unpackHalf2x16(packHalf2x16(val.zz)).x);
}
vec3 quantizeToF16_e8fd14() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = tint_quantize_to_f16(arg_0);
  return res;
}
void main() {
  v.tint_symbol = quantizeToF16_e8fd14();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec3 tint_symbol;
} v;
vec3 tint_quantize_to_f16(vec3 val) {
  vec2 v_1 = unpackHalf2x16(packHalf2x16(val.xy));
  return vec3(v_1, unpackHalf2x16(packHalf2x16(val.zz)).x);
}
vec3 quantizeToF16_e8fd14() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = tint_quantize_to_f16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = quantizeToF16_e8fd14();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

layout(location = 0) flat out vec3 vertex_main_loc0_Output;
vec3 tint_quantize_to_f16(vec3 val) {
  vec2 v = unpackHalf2x16(packHalf2x16(val.xy));
  return vec3(v, unpackHalf2x16(packHalf2x16(val.zz)).x);
}
vec3 quantizeToF16_e8fd14() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = tint_quantize_to_f16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec3(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = quantizeToF16_e8fd14();
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
