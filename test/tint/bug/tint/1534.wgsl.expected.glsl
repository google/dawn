#version 310 es

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

struct g {
  uvec3 a;
  uint pad;
};

struct h {
  uint a;
};

layout(binding = 0, std140) uniform i_block_ubo {
  g inner;
} i;

layout(binding = 1, std430) buffer j_block_ssbo {
  h inner;
} j;

void tint_symbol() {
  uint l = tint_int_dot(i.inner.a, i.inner.a);
  j.inner.a = i.inner.a.x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
