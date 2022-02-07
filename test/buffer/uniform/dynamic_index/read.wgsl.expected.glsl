#version 310 es

struct Inner {
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
  ivec4 k[4];
};

struct S {
  Inner arr[8];
};

layout(binding = 0) uniform S_1 {
  Inner arr[8];
} s;

void tint_symbol(uint idx) {
  ivec3 a = s.arr[idx].a;
  int b = s.arr[idx].b;
  uvec3 c = s.arr[idx].c;
  uint d = s.arr[idx].d;
  vec3 e = s.arr[idx].e;
  float f = s.arr[idx].f;
  ivec2 g = s.arr[idx].g;
  ivec2 h = s.arr[idx].h;
  mat2x3 i = s.arr[idx].i;
  mat3x2 j = s.arr[idx].j;
  ivec4 k[4] = s.arr[idx].k;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
