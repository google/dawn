//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec3 inner;
} v;
uvec3 countLeadingZeros_ab6345() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 v_2 = mix(uvec3(0u), uvec3(16u), lessThanEqual(v_1, uvec3(65535u)));
  uvec3 v_3 = (v_1 << v_2);
  uvec3 v_4 = mix(uvec3(0u), uvec3(8u), lessThanEqual(v_3, uvec3(16777215u)));
  uvec3 v_5 = (v_3 << v_4);
  uvec3 v_6 = mix(uvec3(0u), uvec3(4u), lessThanEqual(v_5, uvec3(268435455u)));
  uvec3 v_7 = (v_5 << v_6);
  uvec3 v_8 = mix(uvec3(0u), uvec3(2u), lessThanEqual(v_7, uvec3(1073741823u)));
  uvec3 v_9 = (v_7 << v_8);
  uvec3 v_10 = mix(uvec3(0u), uvec3(1u), equal(v_9, uvec3(0u)));
  uvec3 res = ((v_2 | (v_4 | (v_6 | (v_8 | (mix(uvec3(0u), uvec3(1u), lessThanEqual(v_9, uvec3(2147483647u))) | v_10))))) + v_10);
  return res;
}
void main() {
  v.inner = countLeadingZeros_ab6345();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 countLeadingZeros_ab6345() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 v_2 = mix(uvec3(0u), uvec3(16u), lessThanEqual(v_1, uvec3(65535u)));
  uvec3 v_3 = (v_1 << v_2);
  uvec3 v_4 = mix(uvec3(0u), uvec3(8u), lessThanEqual(v_3, uvec3(16777215u)));
  uvec3 v_5 = (v_3 << v_4);
  uvec3 v_6 = mix(uvec3(0u), uvec3(4u), lessThanEqual(v_5, uvec3(268435455u)));
  uvec3 v_7 = (v_5 << v_6);
  uvec3 v_8 = mix(uvec3(0u), uvec3(2u), lessThanEqual(v_7, uvec3(1073741823u)));
  uvec3 v_9 = (v_7 << v_8);
  uvec3 v_10 = mix(uvec3(0u), uvec3(1u), equal(v_9, uvec3(0u)));
  uvec3 res = ((v_2 | (v_4 | (v_6 | (v_8 | (mix(uvec3(0u), uvec3(1u), lessThanEqual(v_9, uvec3(2147483647u))) | v_10))))) + v_10);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countLeadingZeros_ab6345();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 tint_interstage_location0;
uvec3 countLeadingZeros_ab6345() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  uvec3 v_1 = mix(uvec3(0u), uvec3(16u), lessThanEqual(v, uvec3(65535u)));
  uvec3 v_2 = (v << v_1);
  uvec3 v_3 = mix(uvec3(0u), uvec3(8u), lessThanEqual(v_2, uvec3(16777215u)));
  uvec3 v_4 = (v_2 << v_3);
  uvec3 v_5 = mix(uvec3(0u), uvec3(4u), lessThanEqual(v_4, uvec3(268435455u)));
  uvec3 v_6 = (v_4 << v_5);
  uvec3 v_7 = mix(uvec3(0u), uvec3(2u), lessThanEqual(v_6, uvec3(1073741823u)));
  uvec3 v_8 = (v_6 << v_7);
  uvec3 v_9 = mix(uvec3(0u), uvec3(1u), equal(v_8, uvec3(0u)));
  uvec3 res = ((v_1 | (v_3 | (v_5 | (v_7 | (mix(uvec3(0u), uvec3(1u), lessThanEqual(v_8, uvec3(2147483647u))) | v_9))))) + v_9);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_10 = VertexOutput(vec4(0.0f), uvec3(0u));
  v_10.pos = vec4(0.0f);
  v_10.prevent_dce = countLeadingZeros_ab6345();
  return v_10;
}
void main() {
  VertexOutput v_11 = vertex_main_inner();
  gl_Position = vec4(v_11.pos.x, -(v_11.pos.y), ((2.0f * v_11.pos.z) - v_11.pos.w), v_11.pos.w);
  tint_interstage_location0 = v_11.prevent_dce;
  gl_PointSize = 1.0f;
}
