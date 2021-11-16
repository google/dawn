#version 310 es
precision mediump float;

struct Inner {
  int x;
};
struct tint_padded_array_element {
  Inner el;
};

layout (binding = 0) buffer S_1 {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  mat2x3 g;
  mat3x2 h;
  Inner i;
  tint_padded_array_element j[4];
} s;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
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
  tint_padded_array_element j[4] = s.j;
  return;
}
void main() {
  tint_symbol();
}


