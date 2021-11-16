#version 310 es
precision mediump float;

struct Inner {
  int x;
};
struct tint_padded_array_element {
  Inner el;
};

layout (binding = 0) uniform S_1 {
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
  tint_padded_array_element l[4];
} s;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
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
  tint_padded_array_element l[4] = s.l;
  return;
}
void main() {
  tint_symbol();
}


