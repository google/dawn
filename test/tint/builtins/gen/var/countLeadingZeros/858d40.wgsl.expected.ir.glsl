#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 countLeadingZeros_858d40() {
  ivec2 arg_0 = ivec2(1);
  uvec2 v_1 = uvec2(arg_0);
  uvec2 v_2 = mix(uvec2(0u), uvec2(16u), lessThanEqual(v_1, uvec2(65535u)));
  uvec2 v_3 = mix(uvec2(0u), uvec2(8u), lessThanEqual((v_1 << v_2), uvec2(16777215u)));
  uvec2 v_4 = mix(uvec2(0u), uvec2(4u), lessThanEqual(((v_1 << v_2) << v_3), uvec2(268435455u)));
  uvec2 v_5 = mix(uvec2(0u), uvec2(2u), lessThanEqual((((v_1 << v_2) << v_3) << v_4), uvec2(1073741823u)));
  uvec2 v_6 = mix(uvec2(0u), uvec2(1u), lessThanEqual(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec2(2147483647u)));
  uvec2 v_7 = mix(uvec2(0u), uvec2(1u), equal(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec2(0u)));
  ivec2 res = ivec2(((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7));
  return res;
}
void main() {
  v.tint_symbol = countLeadingZeros_858d40();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 countLeadingZeros_858d40() {
  ivec2 arg_0 = ivec2(1);
  uvec2 v_1 = uvec2(arg_0);
  uvec2 v_2 = mix(uvec2(0u), uvec2(16u), lessThanEqual(v_1, uvec2(65535u)));
  uvec2 v_3 = mix(uvec2(0u), uvec2(8u), lessThanEqual((v_1 << v_2), uvec2(16777215u)));
  uvec2 v_4 = mix(uvec2(0u), uvec2(4u), lessThanEqual(((v_1 << v_2) << v_3), uvec2(268435455u)));
  uvec2 v_5 = mix(uvec2(0u), uvec2(2u), lessThanEqual((((v_1 << v_2) << v_3) << v_4), uvec2(1073741823u)));
  uvec2 v_6 = mix(uvec2(0u), uvec2(1u), lessThanEqual(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec2(2147483647u)));
  uvec2 v_7 = mix(uvec2(0u), uvec2(1u), equal(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec2(0u)));
  ivec2 res = ivec2(((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = countLeadingZeros_858d40();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec2 prevent_dce;
};

layout(location = 0) flat out ivec2 vertex_main_loc0_Output;
ivec2 countLeadingZeros_858d40() {
  ivec2 arg_0 = ivec2(1);
  uvec2 v = uvec2(arg_0);
  uvec2 v_1 = mix(uvec2(0u), uvec2(16u), lessThanEqual(v, uvec2(65535u)));
  uvec2 v_2 = mix(uvec2(0u), uvec2(8u), lessThanEqual((v << v_1), uvec2(16777215u)));
  uvec2 v_3 = mix(uvec2(0u), uvec2(4u), lessThanEqual(((v << v_1) << v_2), uvec2(268435455u)));
  uvec2 v_4 = mix(uvec2(0u), uvec2(2u), lessThanEqual((((v << v_1) << v_2) << v_3), uvec2(1073741823u)));
  uvec2 v_5 = mix(uvec2(0u), uvec2(1u), lessThanEqual(((((v << v_1) << v_2) << v_3) << v_4), uvec2(2147483647u)));
  uvec2 v_6 = mix(uvec2(0u), uvec2(1u), equal(((((v << v_1) << v_2) << v_3) << v_4), uvec2(0u)));
  ivec2 res = ivec2(((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec2(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_858d40();
  return tint_symbol;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = v_7.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
