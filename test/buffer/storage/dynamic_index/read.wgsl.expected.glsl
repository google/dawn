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
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.idx);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.idx = uint(gl_LocalInvocationIndex);
  tint_symbol(inputs);
}


