#version 310 es

struct Inner {
  int x;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std140) uniform S_std140_ubo {
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
} s;

mat3x2 load_s_j() {
  return mat3x2(s.j_0, s.j_1, s.j_2);
}

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
  mat3x2 j = load_s_j();
  Inner k = s.k;
  Inner l[4] = s.l;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
