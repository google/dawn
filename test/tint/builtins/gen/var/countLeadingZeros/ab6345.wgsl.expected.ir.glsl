#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 countLeadingZeros_ab6345() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 v_2 = mix(uvec3(0u), uvec3(16u), lessThanEqual(v_1, uvec3(65535u)));
  uvec3 v_3 = mix(uvec3(0u), uvec3(8u), lessThanEqual((v_1 << v_2), uvec3(16777215u)));
  uvec3 v_4 = mix(uvec3(0u), uvec3(4u), lessThanEqual(((v_1 << v_2) << v_3), uvec3(268435455u)));
  uvec3 v_5 = mix(uvec3(0u), uvec3(2u), lessThanEqual((((v_1 << v_2) << v_3) << v_4), uvec3(1073741823u)));
  uvec3 v_6 = mix(uvec3(0u), uvec3(1u), lessThanEqual(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec3(2147483647u)));
  uvec3 v_7 = mix(uvec3(0u), uvec3(1u), equal(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec3(0u)));
  uvec3 res = ((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7);
  return res;
}
void main() {
  v.tint_symbol = countLeadingZeros_ab6345();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 countLeadingZeros_ab6345() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 v_2 = mix(uvec3(0u), uvec3(16u), lessThanEqual(v_1, uvec3(65535u)));
  uvec3 v_3 = mix(uvec3(0u), uvec3(8u), lessThanEqual((v_1 << v_2), uvec3(16777215u)));
  uvec3 v_4 = mix(uvec3(0u), uvec3(4u), lessThanEqual(((v_1 << v_2) << v_3), uvec3(268435455u)));
  uvec3 v_5 = mix(uvec3(0u), uvec3(2u), lessThanEqual((((v_1 << v_2) << v_3) << v_4), uvec3(1073741823u)));
  uvec3 v_6 = mix(uvec3(0u), uvec3(1u), lessThanEqual(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec3(2147483647u)));
  uvec3 v_7 = mix(uvec3(0u), uvec3(1u), equal(((((v_1 << v_2) << v_3) << v_4) << v_5), uvec3(0u)));
  uvec3 res = ((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = countLeadingZeros_ab6345();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 countLeadingZeros_ab6345() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  uvec3 v_1 = mix(uvec3(0u), uvec3(16u), lessThanEqual(v, uvec3(65535u)));
  uvec3 v_2 = mix(uvec3(0u), uvec3(8u), lessThanEqual((v << v_1), uvec3(16777215u)));
  uvec3 v_3 = mix(uvec3(0u), uvec3(4u), lessThanEqual(((v << v_1) << v_2), uvec3(268435455u)));
  uvec3 v_4 = mix(uvec3(0u), uvec3(2u), lessThanEqual((((v << v_1) << v_2) << v_3), uvec3(1073741823u)));
  uvec3 v_5 = mix(uvec3(0u), uvec3(1u), lessThanEqual(((((v << v_1) << v_2) << v_3) << v_4), uvec3(2147483647u)));
  uvec3 v_6 = mix(uvec3(0u), uvec3(1u), equal(((((v << v_1) << v_2) << v_3) << v_4), uvec3(0u)));
  uvec3 res = ((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_ab6345();
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
