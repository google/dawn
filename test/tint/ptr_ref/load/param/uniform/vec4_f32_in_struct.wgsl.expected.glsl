#version 310 es

struct str {
  vec4 i;
};

layout(binding = 0, std140) uniform S_block_ubo {
  str inner;
} S;

vec4 func_S_inner_i() {
  return S.inner.i;
}

void tint_symbol() {
  vec4 r = func_S_inner_i();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
