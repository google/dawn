#version 310 es

struct Inner {
  int x;
};

layout(binding = 0, std430) buffer S_ssbo {
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
} s;

void tint_symbol() {
  ivec3 a = s.a;
  int b = s.b;
  uvec3 c = s.c;
  uint d = s.d;
  vec3 e = s.e;
  float f = s.f;
  mat2x3 g = s.g;
  mat3x2 h = s.h;
  Inner i = s.i;
  Inner j[4] = s.j;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
