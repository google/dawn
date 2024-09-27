#version 310 es
precision highp float;
precision highp int;

uvec3 tint_first_trailing_bit(uvec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(16u), uvec3(0u), bvec3((x & uvec3(65535u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(8u), uvec3(0u), bvec3((x & uvec3(255u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(4u), uvec3(0u), bvec3((x & uvec3(15u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(2u), uvec3(0u), bvec3((x & uvec3(3u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(1u), uvec3(0u), bvec3((x & uvec3(1u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return uvec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec3 inner;
} prevent_dce;

uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 res = tint_first_trailing_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = firstTrailingBit_cb51ce();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec3 tint_first_trailing_bit(uvec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(16u), uvec3(0u), bvec3((x & uvec3(65535u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(8u), uvec3(0u), bvec3((x & uvec3(255u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(4u), uvec3(0u), bvec3((x & uvec3(15u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(2u), uvec3(0u), bvec3((x & uvec3(3u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(1u), uvec3(0u), bvec3((x & uvec3(1u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return uvec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec3 inner;
} prevent_dce;

uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 res = tint_first_trailing_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = firstTrailingBit_cb51ce();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

uvec3 tint_first_trailing_bit(uvec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(16u), uvec3(0u), bvec3((x & uvec3(65535u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(8u), uvec3(0u), bvec3((x & uvec3(255u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(4u), uvec3(0u), bvec3((x & uvec3(15u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(2u), uvec3(0u), bvec3((x & uvec3(3u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(1u), uvec3(0u), bvec3((x & uvec3(1u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return uvec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(location = 0) flat out uvec3 prevent_dce_1;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 res = tint_first_trailing_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), uvec3(0u, 0u, 0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstTrailingBit_cb51ce();
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
