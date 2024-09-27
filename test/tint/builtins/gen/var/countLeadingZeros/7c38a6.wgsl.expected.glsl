#version 310 es
precision highp float;
precision highp int;

ivec3 tint_count_leading_zeros(ivec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(0u), uvec3(16u), lessThanEqual(x, uvec3(65535u)));
  x = (x << b16);
  uvec3 b8 = mix(uvec3(0u), uvec3(8u), lessThanEqual(x, uvec3(16777215u)));
  x = (x << b8);
  uvec3 b4 = mix(uvec3(0u), uvec3(4u), lessThanEqual(x, uvec3(268435455u)));
  x = (x << b4);
  uvec3 b2 = mix(uvec3(0u), uvec3(2u), lessThanEqual(x, uvec3(1073741823u)));
  x = (x << b2);
  uvec3 b1 = mix(uvec3(0u), uvec3(1u), lessThanEqual(x, uvec3(2147483647u)));
  uvec3 is_zero = mix(uvec3(0u), uvec3(1u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec3 inner;
} prevent_dce;

ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = countLeadingZeros_7c38a6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

ivec3 tint_count_leading_zeros(ivec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(0u), uvec3(16u), lessThanEqual(x, uvec3(65535u)));
  x = (x << b16);
  uvec3 b8 = mix(uvec3(0u), uvec3(8u), lessThanEqual(x, uvec3(16777215u)));
  x = (x << b8);
  uvec3 b4 = mix(uvec3(0u), uvec3(4u), lessThanEqual(x, uvec3(268435455u)));
  x = (x << b4);
  uvec3 b2 = mix(uvec3(0u), uvec3(2u), lessThanEqual(x, uvec3(1073741823u)));
  x = (x << b2);
  uvec3 b1 = mix(uvec3(0u), uvec3(1u), lessThanEqual(x, uvec3(2147483647u)));
  uvec3 is_zero = mix(uvec3(0u), uvec3(1u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec3 inner;
} prevent_dce;

ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = countLeadingZeros_7c38a6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

ivec3 tint_count_leading_zeros(ivec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(0u), uvec3(16u), lessThanEqual(x, uvec3(65535u)));
  x = (x << b16);
  uvec3 b8 = mix(uvec3(0u), uvec3(8u), lessThanEqual(x, uvec3(16777215u)));
  x = (x << b8);
  uvec3 b4 = mix(uvec3(0u), uvec3(4u), lessThanEqual(x, uvec3(268435455u)));
  x = (x << b4);
  uvec3 b2 = mix(uvec3(0u), uvec3(2u), lessThanEqual(x, uvec3(1073741823u)));
  x = (x << b2);
  uvec3 b1 = mix(uvec3(0u), uvec3(1u), lessThanEqual(x, uvec3(2147483647u)));
  uvec3 is_zero = mix(uvec3(0u), uvec3(1u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(location = 0) flat out ivec3 prevent_dce_1;
ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), ivec3(0, 0, 0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_7c38a6();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
