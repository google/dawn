#version 310 es

struct Inner {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  mat2x3 g;
  mat3x2 h;
  uint pad;
  uint pad_1;
  ivec4 i[4];
};

layout(binding = 0, std430) buffer S_ssbo {
  Inner arr[];
} s;

void tint_symbol(uint idx) {
  ivec3 a = s.arr[idx].a;
  int b = s.arr[idx].b;
  uvec3 c = s.arr[idx].c;
  uint d = s.arr[idx].d;
  vec3 e = s.arr[idx].e;
  float f = s.arr[idx].f;
  mat2x3 g = s.arr[idx].g;
  mat3x2 h = s.arr[idx].h;
  ivec4 i[4] = s.arr[idx].i;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
