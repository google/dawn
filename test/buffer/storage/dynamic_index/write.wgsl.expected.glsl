#version 310 es
precision mediump float;

struct Inner {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  mat2x3 g;
  mat3x2 h;
  ivec4 i[4];
};

layout (binding = 0) buffer S_1 {
  Inner arr[];
} s;

struct tint_symbol_2 {
  uint idx;
};

void tint_symbol_inner(uint idx) {
  s.arr[idx].a = ivec3(0, 0, 0);
  s.arr[idx].b = 0;
  s.arr[idx].c = uvec3(0u, 0u, 0u);
  s.arr[idx].d = 0u;
  s.arr[idx].e = vec3(0.0f, 0.0f, 0.0f);
  s.arr[idx].f = 0.0f;
  s.arr[idx].g = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  s.arr[idx].h = mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  ivec4 tint_symbol_3[4] = ivec4[4](ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0));
  s.arr[idx].i = tint_symbol_3;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.idx);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.idx = uint(gl_LocalInvocationIndex);
  tint_symbol(inputs);
}


