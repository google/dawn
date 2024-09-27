#version 310 es
precision highp float;
precision highp int;

int tint_count_leading_zeros(int v) {
  uint x = uint(v);
  uint b16 = mix(0u, 16u, (x <= 65535u));
  x = (x << b16);
  uint b8 = mix(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  uint b4 = mix(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  uint b2 = mix(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  uint b1 = mix(0u, 1u, (x <= 2147483647u));
  uint is_zero = mix(0u, 1u, (x == 0u));
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

int countLeadingZeros_6d4656() {
  int arg_0 = 1;
  int res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = countLeadingZeros_6d4656();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

int tint_count_leading_zeros(int v) {
  uint x = uint(v);
  uint b16 = mix(0u, 16u, (x <= 65535u));
  x = (x << b16);
  uint b8 = mix(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  uint b4 = mix(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  uint b2 = mix(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  uint b1 = mix(0u, 1u, (x <= 2147483647u));
  uint is_zero = mix(0u, 1u, (x == 0u));
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

int countLeadingZeros_6d4656() {
  int arg_0 = 1;
  int res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

void compute_main() {
  prevent_dce.inner = countLeadingZeros_6d4656();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

int tint_count_leading_zeros(int v) {
  uint x = uint(v);
  uint b16 = mix(0u, 16u, (x <= 65535u));
  x = (x << b16);
  uint b8 = mix(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  uint b4 = mix(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  uint b2 = mix(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  uint b1 = mix(0u, 1u, (x <= 2147483647u));
  uint is_zero = mix(0u, 1u, (x == 0u));
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(location = 0) flat out int prevent_dce_1;
int countLeadingZeros_6d4656() {
  int arg_0 = 1;
  int res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_6d4656();
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
