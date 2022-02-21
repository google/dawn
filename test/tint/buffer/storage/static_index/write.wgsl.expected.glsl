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
};

layout(binding = 0, std430) buffer S_1 {
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
} s;
void tint_symbol() {
  s.a = ivec3(0, 0, 0);
  s.b = 0;
  s.c = uvec3(0u, 0u, 0u);
  s.d = 0u;
  s.e = vec3(0.0f, 0.0f, 0.0f);
  s.f = 0.0f;
  s.g = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  s.h = mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  Inner tint_symbol_1 = Inner(0);
  s.i = tint_symbol_1;
  Inner tint_symbol_2[4] = Inner[4](Inner(0), Inner(0), Inner(0), Inner(0));
  s.j = tint_symbol_2;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
