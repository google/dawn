#version 310 es


struct S {
  vec3 v;
  uint i;
};

layout(binding = 0, std430)
buffer io_block_1_ssbo {
  S inner;
} v_1;
vec3 Bad(uint index, vec3 rd) {
  vec3 normal = vec3(0.0f);
  normal[index] = -(sign(rd[index]));
  return normalize(normal);
}
void tint_symbol_inner(uint idx) {
  v_1.inner.v = Bad(v_1.inner.i, v_1.inner.v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
