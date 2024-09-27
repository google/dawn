#version 310 es
precision highp float;
precision highp int;

uvec2 tint_count_trailing_zeros(uvec2 v) {
  uvec2 x = uvec2(v);
  uvec2 b16 = mix(uvec2(16u), uvec2(0u), bvec2((x & uvec2(65535u))));
  x = (x >> b16);
  uvec2 b8 = mix(uvec2(8u), uvec2(0u), bvec2((x & uvec2(255u))));
  x = (x >> b8);
  uvec2 b4 = mix(uvec2(4u), uvec2(0u), bvec2((x & uvec2(15u))));
  x = (x >> b4);
  uvec2 b2 = mix(uvec2(2u), uvec2(0u), bvec2((x & uvec2(3u))));
  x = (x >> b2);
  uvec2 b1 = mix(uvec2(1u), uvec2(0u), bvec2((x & uvec2(1u))));
  uvec2 is_zero = mix(uvec2(0u), uvec2(1u), equal(x, uvec2(0u)));
  return uvec2((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

uvec2 countTrailingZeros_1ad138() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = countTrailingZeros_1ad138();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec2 tint_count_trailing_zeros(uvec2 v) {
  uvec2 x = uvec2(v);
  uvec2 b16 = mix(uvec2(16u), uvec2(0u), bvec2((x & uvec2(65535u))));
  x = (x >> b16);
  uvec2 b8 = mix(uvec2(8u), uvec2(0u), bvec2((x & uvec2(255u))));
  x = (x >> b8);
  uvec2 b4 = mix(uvec2(4u), uvec2(0u), bvec2((x & uvec2(15u))));
  x = (x >> b4);
  uvec2 b2 = mix(uvec2(2u), uvec2(0u), bvec2((x & uvec2(3u))));
  x = (x >> b2);
  uvec2 b1 = mix(uvec2(1u), uvec2(0u), bvec2((x & uvec2(1u))));
  uvec2 is_zero = mix(uvec2(0u), uvec2(1u), equal(x, uvec2(0u)));
  return uvec2((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

uvec2 countTrailingZeros_1ad138() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = countTrailingZeros_1ad138();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

uvec2 tint_count_trailing_zeros(uvec2 v) {
  uvec2 x = uvec2(v);
  uvec2 b16 = mix(uvec2(16u), uvec2(0u), bvec2((x & uvec2(65535u))));
  x = (x >> b16);
  uvec2 b8 = mix(uvec2(8u), uvec2(0u), bvec2((x & uvec2(255u))));
  x = (x >> b8);
  uvec2 b4 = mix(uvec2(4u), uvec2(0u), bvec2((x & uvec2(15u))));
  x = (x >> b4);
  uvec2 b2 = mix(uvec2(2u), uvec2(0u), bvec2((x & uvec2(3u))));
  x = (x >> b2);
  uvec2 b1 = mix(uvec2(1u), uvec2(0u), bvec2((x & uvec2(1u))));
  uvec2 is_zero = mix(uvec2(0u), uvec2(1u), equal(x, uvec2(0u)));
  return uvec2((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(location = 0) flat out uvec2 prevent_dce_1;
uvec2 countTrailingZeros_1ad138() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), uvec2(0u, 0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countTrailingZeros_1ad138();
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
