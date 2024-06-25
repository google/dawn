#version 310 es
precision highp float;
precision highp int;

uvec4 tint_unpack_4xu8(uint a) {
  uvec4 a_vec4u = (uvec4(a) >> uvec4(0u, 8u, 16u, 24u));
  return (a_vec4u & uvec4(255u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uvec4 res = tint_unpack_4xu8(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = unpack4xU8_a5ea55();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec4 tint_unpack_4xu8(uint a) {
  uvec4 a_vec4u = (uvec4(a) >> uvec4(0u, 8u, 16u, 24u));
  return (a_vec4u & uvec4(255u));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uvec4 res = tint_unpack_4xu8(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = unpack4xU8_a5ea55();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

uvec4 tint_unpack_4xu8(uint a) {
  uvec4 a_vec4u = (uvec4(a) >> uvec4(0u, 8u, 16u, 24u));
  return (a_vec4u & uvec4(255u));
}

layout(location = 0) flat out uvec4 prevent_dce_1;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uvec4 res = tint_unpack_4xu8(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), uvec4(0u, 0u, 0u, 0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = unpack4xU8_a5ea55();
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
