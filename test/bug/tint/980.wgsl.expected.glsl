#version 310 es
precision mediump float;

vec3 Bad(uint index, vec3 rd) {
  vec3 normal = vec3(0.0f);
  normal[index] = -(sign(rd[index]));
  return normalize(normal);
}

layout (binding = 0) buffer S_1 {
  vec3 v;
  uint i;
} io;

struct tint_symbol_2 {
  uint idx;
};

void tint_symbol_inner(uint idx) {
  io.v = Bad(io.i, io.v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.idx);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.idx = uint(gl_LocalInvocationIndex);
  tint_symbol(inputs);
}


