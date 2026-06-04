//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec4 inner;
} v;
ivec4 countLeadingZeros_eab32b() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v_1 = uvec4(arg_0);
  uvec4 v_2 = mix(uvec4(0u), uvec4(16u), lessThanEqual(v_1, uvec4(65535u)));
  uvec4 v_3 = (v_1 << v_2);
  uvec4 v_4 = mix(uvec4(0u), uvec4(8u), lessThanEqual(v_3, uvec4(16777215u)));
  uvec4 v_5 = (v_3 << v_4);
  uvec4 v_6 = mix(uvec4(0u), uvec4(4u), lessThanEqual(v_5, uvec4(268435455u)));
  uvec4 v_7 = (v_5 << v_6);
  uvec4 v_8 = mix(uvec4(0u), uvec4(2u), lessThanEqual(v_7, uvec4(1073741823u)));
  uvec4 v_9 = (v_7 << v_8);
  uvec4 v_10 = mix(uvec4(0u), uvec4(1u), equal(v_9, uvec4(0u)));
  ivec4 res = ivec4(((v_2 | (v_4 | (v_6 | (v_8 | (mix(uvec4(0u), uvec4(1u), lessThanEqual(v_9, uvec4(2147483647u))) | v_10))))) + v_10));
  return res;
}
void main() {
  v.inner = countLeadingZeros_eab32b();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
ivec4 countLeadingZeros_eab32b() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v_1 = uvec4(arg_0);
  uvec4 v_2 = mix(uvec4(0u), uvec4(16u), lessThanEqual(v_1, uvec4(65535u)));
  uvec4 v_3 = (v_1 << v_2);
  uvec4 v_4 = mix(uvec4(0u), uvec4(8u), lessThanEqual(v_3, uvec4(16777215u)));
  uvec4 v_5 = (v_3 << v_4);
  uvec4 v_6 = mix(uvec4(0u), uvec4(4u), lessThanEqual(v_5, uvec4(268435455u)));
  uvec4 v_7 = (v_5 << v_6);
  uvec4 v_8 = mix(uvec4(0u), uvec4(2u), lessThanEqual(v_7, uvec4(1073741823u)));
  uvec4 v_9 = (v_7 << v_8);
  uvec4 v_10 = mix(uvec4(0u), uvec4(1u), equal(v_9, uvec4(0u)));
  ivec4 res = ivec4(((v_2 | (v_4 | (v_6 | (v_8 | (mix(uvec4(0u), uvec4(1u), lessThanEqual(v_9, uvec4(2147483647u))) | v_10))))) + v_10));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countLeadingZeros_eab32b();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 countLeadingZeros_eab32b() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v = uvec4(arg_0);
  uvec4 v_1 = mix(uvec4(0u), uvec4(16u), lessThanEqual(v, uvec4(65535u)));
  uvec4 v_2 = (v << v_1);
  uvec4 v_3 = mix(uvec4(0u), uvec4(8u), lessThanEqual(v_2, uvec4(16777215u)));
  uvec4 v_4 = (v_2 << v_3);
  uvec4 v_5 = mix(uvec4(0u), uvec4(4u), lessThanEqual(v_4, uvec4(268435455u)));
  uvec4 v_6 = (v_4 << v_5);
  uvec4 v_7 = mix(uvec4(0u), uvec4(2u), lessThanEqual(v_6, uvec4(1073741823u)));
  uvec4 v_8 = (v_6 << v_7);
  uvec4 v_9 = mix(uvec4(0u), uvec4(1u), equal(v_8, uvec4(0u)));
  ivec4 res = ivec4(((v_1 | (v_3 | (v_5 | (v_7 | (mix(uvec4(0u), uvec4(1u), lessThanEqual(v_8, uvec4(2147483647u))) | v_9))))) + v_9));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_10 = VertexOutput(vec4(0.0f), ivec4(0));
  v_10.pos = vec4(0.0f);
  v_10.prevent_dce = countLeadingZeros_eab32b();
  return v_10;
}
void main() {
  VertexOutput v_11 = vertex_main_inner();
  gl_Position = vec4(v_11.pos.x, -(v_11.pos.y), ((2.0f * v_11.pos.z) - v_11.pos.w), v_11.pos.w);
  tint_interstage_location0 = v_11.prevent_dce;
  gl_PointSize = 1.0f;
}
