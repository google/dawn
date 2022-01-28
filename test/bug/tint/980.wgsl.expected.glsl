#version 310 es
precision mediump float;

vec3 Bad(uint index, vec3 rd) {
  vec3 normal = vec3(0.0f);
  normal[index] = -(sign(rd[index]));
  return normalize(normal);
}

struct S {
  vec3 v;
  uint i;
};

layout(binding = 0) buffer S_1 {
  vec3 v;
  uint i;
} io;
void tint_symbol(uint idx) {
  io.v = Bad(io.i, io.v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
