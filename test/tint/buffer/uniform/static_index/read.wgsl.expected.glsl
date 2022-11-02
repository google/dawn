#version 310 es

struct Inner {
  int x;
  uint pad;
  uint pad_1;
  uint pad_2;
};

struct S {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  ivec2 g;
  ivec2 h;
  mat2x3 i;
  mat3x2 j;
  uint pad_3;
  uint pad_4;
  Inner k;
  Inner l[4];
};

struct S_std140 {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  ivec2 g;
  ivec2 h;
  mat2x3 i;
  vec2 j_0;
  vec2 j_1;
  vec2 j_2;
  uint pad_3;
  uint pad_4;
  Inner k;
  Inner l[4];
};

layout(binding = 0, std140) uniform s_block_std140_ubo {
  S_std140 inner;
} s;

mat3x2 load_s_inner_j() {
  return mat3x2(s.inner.j_0, s.inner.j_1, s.inner.j_2);
}

void tint_symbol() {
  ivec3 a = s.inner.a;
  int b = s.inner.b;
  uvec3 c = s.inner.c;
  uint d = s.inner.d;
  vec3 e = s.inner.e;
  float f = s.inner.f;
  ivec2 g = s.inner.g;
  ivec2 h = s.inner.h;
  mat2x3 i = s.inner.i;
  mat3x2 j = load_s_inner_j();
  Inner k = s.inner.k;
  Inner l[4] = s.inner.l;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
