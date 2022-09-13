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
  s.arr[idx].a = ivec3(0);
  s.arr[idx].b = 0;
  s.arr[idx].c = uvec3(0u);
  s.arr[idx].d = 0u;
  s.arr[idx].e = vec3(0.0f);
  s.arr[idx].f = 0.0f;
  s.arr[idx].g = mat2x3(vec3(0.0f), vec3(0.0f));
  s.arr[idx].h = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  ivec4 tint_symbol_1[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  s.arr[idx].i = tint_symbol_1;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
