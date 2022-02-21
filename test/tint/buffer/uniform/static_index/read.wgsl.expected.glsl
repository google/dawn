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
  ivec2 g;
  ivec2 h;
  mat2x3 i;
  mat3x2 j;
  Inner k;
  Inner l[4];
};

layout(binding = 0) uniform S_1 {
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
  Inner k;
  Inner l[4];
} s;

void tint_symbol() {
  ivec3 a = s.a;
  int b = s.b;
  uvec3 c = s.c;
  uint d = s.d;
  vec3 e = s.e;
  float f = s.f;
  ivec2 g = s.g;
  ivec2 h = s.h;
  mat2x3 i = s.i;
  mat3x2 j = s.j;
  Inner k = s.k;
  Inner l[4] = s.l;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
