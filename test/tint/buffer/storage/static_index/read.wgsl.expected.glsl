#version 310 es

struct Inner {
  int x;
};

struct S {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  mat2x3 g;
  mat3x2 h;
  Inner i;
  Inner j[4];
  uint pad;
};

layout(binding = 0, std430) buffer s_block_ssbo {
  S inner;
} s;

void tint_symbol() {
  ivec3 a = s.inner.a;
  int b = s.inner.b;
  uvec3 c = s.inner.c;
  uint d = s.inner.d;
  vec3 e = s.inner.e;
  float f = s.inner.f;
  mat2x3 g = s.inner.g;
  mat3x2 h = s.inner.h;
  Inner i = s.inner.i;
  Inner j[4] = s.inner.j;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
