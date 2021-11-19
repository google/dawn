#version 310 es
precision mediump float;

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

layout (binding = 0) uniform S_1 {
  Inner arr[8];
} s;

struct tint_symbol_2 {
  uint idx;
};

void tint_symbol_inner(uint idx) {
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
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.idx);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.idx = uint(gl_LocalInvocationIndex);
  tint_symbol(inputs);
}


