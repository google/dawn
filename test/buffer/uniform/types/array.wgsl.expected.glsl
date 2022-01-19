#version 310 es
precision mediump float;

struct u_block {
  vec4 inner[4];
};

layout (binding = 0) uniform u_block_1 {
  vec4 inner[4];
} u;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  vec4 x[4] = u.inner;
  return;
}
void main() {
  tint_symbol();
}


