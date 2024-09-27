#version 310 es
precision highp float;
precision highp int;

uint tint_first_leading_bit(uint v) {
  uint x = v;
  uint b16 = mix(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  uint b8 = mix(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  uint b4 = mix(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  uint b2 = mix(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  uint b1 = mix(0u, 1u, bool((x & 2u)));
  uint is_zero = mix(0u, 4294967295u, (x == 0u));
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = firstLeadingBit_f0779d();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_first_leading_bit(uint v) {
  uint x = v;
  uint b16 = mix(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  uint b8 = mix(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  uint b4 = mix(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  uint b2 = mix(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  uint b1 = mix(0u, 1u, bool((x & 2u)));
  uint is_zero = mix(0u, 4294967295u, (x == 0u));
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void compute_main() {
  prevent_dce.inner = firstLeadingBit_f0779d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

uint tint_first_leading_bit(uint v) {
  uint x = v;
  uint b16 = mix(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  uint b8 = mix(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  uint b4 = mix(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  uint b2 = mix(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  uint b1 = mix(0u, 1u, bool((x & 2u)));
  uint is_zero = mix(0u, 4294967295u, (x == 0u));
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(location = 0) flat out uint prevent_dce_1;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_f0779d();
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
